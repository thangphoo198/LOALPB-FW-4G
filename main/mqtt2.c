/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include "MQTTClient.h"
#include "ql_data_call.h"
#include "ql_application.h"
#include "sockets.h"
#include "ql_ssl_hal.h"
#include "cJSON.h"
#include "http_fota.h"
// #include "mqtt.h"
#define mqtt_exam_log(fmt, ...) printf("[MQTT][%s, %d] " fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define PROFILE_IDX 1
#define SERVER_DOMAIN "mqtts.ddpham216.com"
#define MQTT_USERNAME
#define MQTT_PASS

#define SSL_ENABLE 1

#if SSL_ENABLE
#define SSL_VERIFY_MODE 1 // SSL_VERIFY_MODE_OPTIONAL
#define SSL_CERT_FROM 0	  // SSL_CERT_FROM_BUF
#define SSL_CIPHER_LIST "ALL"
#define SERVER_PORT_VERIFY_SERVER_ONLY 8307
#define SERVER_PORT_VERIFY_BOTH 8308
#define SERVER_PORT 8883
#else
#define SERVER_PORT 8306
#endif

MQTTClient client = {0};
#if (SERVER_PORT == 8307)
#if (SSL_CERT_FROM == 0)
const static SSLCertPathPtr rootCA_path =
	"\
-----BEGIN CERTIFICATE-----\n\
MIIEhDCCAuwCCQDuE1BpeAeMwzANBgkqhkiG9w0BAQsFADCBgjELMAkGA1UEBhMC\n\
Q04xCzAJBgNVBAgMAkFIMQswCQYDVQQHDAJIRjEQMA4GA1UECgwHUVVFQ1RFTDEL\n\
MAkGA1UECwwCU1QxFjAUBgNVBAMMDTExMi4zMS44NC4xNjQxIjAgBgkqhkiG9w0B\n\
CQEWE2VkZGllLnpoYW5nQHF1ZWN0ZWwwIBcNMjIwMTI1MDcyMzI3WhgPMjEyMjAx\n\
MDEwNzIzMjdaMIGCMQswCQYDVQQGEwJDTjELMAkGA1UECAwCQUgxCzAJBgNVBAcM\n\
AkhGMRAwDgYDVQQKDAdRVUVDVEVMMQswCQYDVQQLDAJTVDEWMBQGA1UEAwwNMTEy\n\
LjMxLjg0LjE2NDEiMCAGCSqGSIb3DQEJARYTZWRkaWUuemhhbmdAcXVlY3RlbDCC\n\
AaIwDQYJKoZIhvcNAQEBBQADggGPADCCAYoCggGBAMt3cjY0eLEDqv8Y7FomA+7N\n\
G5ztAbR7+P/WxjPlodqRDZ5HQORkfAr44gAZcWsKoo4DHTInwr9JBbBnETBMnL8+\n\
13h1PRp5CfwXKFvjppWYvBZfeTwhWQYbSMKINoS+d1Zl11jg/+ZbSd7Fi0bYq8ip\n\
Hbt30H+NANQZP1XQdsCf5/kvn+vXiP4EgJc56JQ9L6ALIF2Q6F3G/PTaYItg463N\n\
lv/S+eRi1VMDSs8Qc+DTlVwlgZZJdSlC8Yjr5pVqoyXm8ENKfSTrdhrLiKSWJTz9\n\
JUr04E7SJ+CoBAnLYNPHR2y0CFS/15aCa1JbK27ZJ/0cvBvzpWdkcgrDtKIcxNYM\n\
9QFPpehb1N4pgqi0NPhCkc/BasfmXUaTwM4ghhi4tQRptKMdTN/kdyC+V5a8Hyhb\n\
Nvw5qeJlLJKpgZ9X3HQzuKstKMkxLNuDIzK9TvO7zLowr+0BetUdllq+fDjXQM0M\n\
+9P3Xv2VmDwGRkmZ0IjYpDjm+qqGTFVLzzVwEqVD6wIDAQABMA0GCSqGSIb3DQEB\n\
CwUAA4IBgQAuNVwkBhd5nyWMmV/ESNxy59Sz+5FcesGclKjs4YocgcKbLD2bS+LN\n\
lKk6zenES7Cq6+l3NMAxxh/QhgHUCThAfREzfPXbmiicrUfaudN4YFivpoFwKIAs\n\
NczsL9S3FPbzAB4nLDATacc2BK0//aKMOU2t3KLNNomKbzlR+EW3wd0F1GoZ9SY6\n\
sCQeLa8Wp1KarOmbvgoFL/DAiTSqjjsU/Lq24dOCCctmG+qXRZxQa4npHD4xJwQJ\n\
qzA0JLu4n+DgoJftm1KpvB0wuzTn6M9+wnk5rv/fGc2t4Zra8B4prEReZZVfy65d\n\
cb8pBdb20Yrmznj+6DR50X/o/8Qzoyj9XpxtjwF23ql0XPYCI7kB03Ms9euP0btc\n\
HFacHapm0qBKx+vWy0V2Qf482OWSbewqaRbud44sErNoKqpqm02yN8PpsCywpFUj\n\
UC5G5DzxzYspMzQv/yidti0scMSKFObseZmNGlRYymCWhXnxmoCFjLpw5RnJSB2+\n\
cZ/1KFFHHZI=\n\
-----END CERTIFICATE-----\n\
";
#endif
#elif (SERVER_PORT == 8883)
#if (SSL_CERT_FROM == 0)
const static SSLCertPathPtr rootCA_path =
	"\
-----BEGIN CERTIFICATE-----\n\
MIIECzCCAvOgAwIBAgIULX5zxIpI/IXFTYgvC1MyGkmxrWgwDQYJKoZIhvcNAQEL\n\
BQAwgZQxCzAJBgNVBAYTAlZOMRAwDgYDVQQIDAdWaWV0bmFtMQ4wDAYDVQQHDAVI\n\
YW5vaTEhMB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMRwwGgYDVQQD\n\
DBNtcXR0cy5kZHBoYW0yMTYuY29tMSIwIAYJKoZIhvcNAQkBFhNkZHBoYW0yMTZA\n\
Z21haWwuY29tMB4XDTI0MTEyODExNDU1NloXDTI3MTEyODExNDU1NlowgZQxCzAJ\n\
BgNVBAYTAlZOMRAwDgYDVQQIDAdWaWV0bmFtMQ4wDAYDVQQHDAVIYW5vaTEhMB8G\n\
A1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMRwwGgYDVQQDDBNtcXR0cy5k\n\
ZHBoYW0yMTYuY29tMSIwIAYJKoZIhvcNAQkBFhNkZHBoYW0yMTZAZ21haWwuY29t\n\
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAteCumWCR+qytFs8sU5TF\n\
KdOak1m28sWA+pSCHXl8/PsbUxvG+yxO263j2XoZmz6lbG4k61dptnebbylP01H1\n\
vEq7g2ghv3HlYsmyNK2sCkMlBRx82yqaAos3Jm55cYKDspb4FD3reln7IL9vv5iu\n\
8JvFeWhWKBipl1DCJ7MzYLm1cqA0b+sM0JD+W5cRvPHcfKSTeXH6cpf36i5+TFa0\n\
V59Id4v+XY83/31Nadi4M9eap7aC5qB5rK6IfGpfaU9M+0Z022WoX6GCfCFepYp7\n\
PYXyiFhC5ldkademshEqUvaZMrTcvdv3GDnswS86uf2TXbtAsRAmug8k6N0lIHmA\n\
zwIDAQABo1MwUTAdBgNVHQ4EFgQUWkH0rbc7s66dvWdbPmiykf/83dwwHwYDVR0j\n\
BBgwFoAUWkH0rbc7s66dvWdbPmiykf/83dwwDwYDVR0TAQH/BAUwAwEB/zANBgkq\n\
hkiG9w0BAQsFAAOCAQEAiTlO0WP1Ud0Djuwgq3PCN/H5BZpvjlSln/k747V2sXBa\n\
21A7azewlUcWp4nsbdJc+ipzXHCqh0Ia7ZVXPIQfuQpO1PrrkOlbgQY0MVClGm9X\n\
wcEM19ToPvMA7aXz7jG+UW5zA19gVJnj7RMyYQMl7l49WACiNawvtn3KrQQaBsJ3\n\
xB5hY6uTvXUrtczgmh50tzAj7ZwlUOU3TXIxCUse6AqvbRM9Zb2WztmKLSK8A5eN\n\
3k5O1Zd1YQbQhb0ZHjCM5SB1D6fIttmaetobDppGerUYrq/WF0yEe0yJYqJw8zeJ\n\
+uZK+zq2mXMPZk+ORQnh+c4v5Z7IMTv31Kyfod48Pg==\n\
-----END CERTIFICATE-----\n\
";
const static SSLCertPathPtr clientCert_path =
	"\
-----BEGIN CERTIFICATE-----\n\
MIIDrjCCApYCFDB0ium2mcZ3tZztAbzYzN8V3ApYMA0GCSqGSIb3DQEBCwUAMIGU\n\
MQswCQYDVQQGEwJWTjEQMA4GA1UECAwHVmlldG5hbTEOMAwGA1UEBwwFSGFub2kx\n\
ITAfBgNVBAoMGEludGVybmV0IFdpZGdpdHMgUHR5IEx0ZDEcMBoGA1UEAwwTbXF0\n\
dHMuZGRwaGFtMjE2LmNvbTEiMCAGCSqGSIb3DQEJARYTZGRwaGFtMjE2QGdtYWls\n\
LmNvbTAeFw0yNDExMjgxMjQxMTVaFw0yNzExMjgxMjQxMTVaMIGRMQswCQYDVQQG\n\
EwJWTjEOMAwGA1UECAwFSGFub2kxDjAMBgNVBAcMBUhhbm9pMQwwCgYDVQQKDANE\n\
RFAxDDAKBgNVBAsMA0REUDEcMBoGA1UEAwwTbXF0dHMuZGRwaGFtMjE2LmNvbTEo\n\
MCYGCSqGSIb3DQEJARYZZC5hY291c3RpYy4yMTZAZ21haWxsLmNvbTCCASIwDQYJ\n\
KoZIhvcNAQEBBQADggEPADCCAQoCggEBAKZW00XEhQJy0nmmqvWHARqYiztyCOHW\n\
7QjHisz9b4ZGQz7cPq0vePKaajU9XyyDerP0s+Lvb1tYmUtlaYyjqDX9BpBpuz7A\n\
vWIgEX5UkolGZ7HfmyheTTATSd3s8r4edP9j0+qrSShHOvRupxl7Nsyyr1y6Cpk2\n\
hNim5608GXpNPG2V3nDWLE7aBhIEGjm69AQdnYWXORmxrp51JBI8p9JhlzDUJ9EG\n\
A79FLP1JibR6UAc5yJ18vzm3T+Bm2DWER63EHo953mgUYI5JgQvNaSR/s90CHlV1\n\
M3hnd6MNAiEPHNEUVEEogqHvNdWHtQmCUf2XDzQvqo3hzndAX8u7aIsCAwEAATAN\n\
BgkqhkiG9w0BAQsFAAOCAQEASFpaI9OE7mwcCSVr37f2k2V3y8XNpHx5NZ1x1VyL\n\
jcafRWBMDWYfJf4RokxSOIy+fze08WfRlyQLxJKig+aUmTBqzBT96Go4YUaWOnNj\n\
UevQfLEsyJ1HZKmEeNZTQic4UzmHl8iXGELtcng1IEVYsdcxJMnW2rjDu9SqovCC\n\
C+Zwwk9rjLiZCWYdiKxEEsVZivIYSbi1hUwyUKoJAF54ZG9NR66Ad8ML9Dq1qDGh\n\
VB/168BFtGcoZbFnQEmu+cr/daLtL8aLrKxUxbQr8i8CJ8KFaBOx0ZCP4bYqSLn/\n\
ocHsh+AaerA6jk/Hn4h1iMfN1k5QuvllIXYxN39ITY5q9w==\n\
-----END CERTIFICATE-----\n\
";
const static SSLCertPathPtr clientKey_path =
	"\
-----BEGIN PRIVATE KEY-----\n\
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQCmVtNFxIUCctJ5\n\
pqr1hwEamIs7cgjh1u0Ix4rM/W+GRkM+3D6tL3jymmo1PV8sg3qz9LPi729bWJlL\n\
ZWmMo6g1/QaQabs+wL1iIBF+VJKJRmex35soXk0wE0nd7PK+HnT/Y9Pqq0koRzr0\n\
bqcZezbMsq9cugqZNoTYpuetPBl6TTxtld5w1ixO2gYSBBo5uvQEHZ2FlzkZsa6e\n\
dSQSPKfSYZcw1CfRBgO/RSz9SYm0elAHOcidfL85t0/gZtg1hEetxB6Ped5oFGCO\n\
SYELzWkkf7PdAh5VdTN4Z3ejDQIhDxzRFFRBKIKh7zXVh7UJglH9lw80L6qN4c53\n\
QF/Lu2iLAgMBAAECggEATs/Jp39O1BOqROhijRWrwLmxEnfb3IFUarn8ulaEHlcx\n\
Kln5Ol5JTI08MECFVRZwr04MY0Q1+05Vx4LjyTaLjxT1JLxr8vYoZRMzpqsUiQU6\n\
QW1kWvKfWN1yvLs1Yj4VCiJ2+HOypGpndQPumWSnWMJ60xD0z5YkyHAASjS1D5xA\n\
KudGQmpma9sTM7X9a6CylWoDZYiSTCNSu3TJ00xSoLtg2b7xAU3guBJNLScUmqEw\n\
03pg93iZjLp7WwY+jfPAEGBTkfIhJp7FVwIio/3gXjMG2yivGln5z13ELVkFJ3Qi\n\
YyG3aMd4pcvSUm8A3oS/c5H2veGskHaWmf10qfjRcQKBgQDRAcsO5boet6jNF1yH\n\
KziBptzs5ebzBu8LzwSjhNGBKMh4lZ5ixYLe40VwyQfmYUP8f5EAcEwW+b/JjnPw\n\
ql4SdLLzOj7NLBCywaZ+FKM2ASl6rGkPEaQ5/+jCE53Dkg7nleW1C76qpGQxP/ey\n\
j7d8QX0d1S+XDgAG1sFg8PZEUQKBgQDLvR6I+WKLNTqFkSMwfXteAP5IefZtK6QL\n\
25VPmcvXci79Ny/cTODzzCx5rkIxPvlWxMCkP7GqOMC4NOx5JD+v3oUjnSxuyhpC\n\
s+VHUYkMWg44gyPiZyN+w0dsEJ9GqspKhBUDumeC21qZBq5K+PCsNIxSmSPpJ8PP\n\
nGgJrhX0GwKBgAYFZzQmqhX4dIR0B9ZdCGOzyumINf8KVBii/h831FFEO4I9zTJL\n\
PlntcjKtl+yrIKpFgofgtFB1IXFNCUoFnnAprH4VT3nZqyF7MrCvJ4SGxAskrGIi\n\
v/hrxujQZ9saehNx3gw5KmUAF2LT1O7v9Pk/abtSSOyxGo8X4LpKNs4BAoGBAJWt\n\
a8hvhnEE+uhzlymhlkeabTWrvFm5zPwVJt1i/LlcHp9Ssuui40TYw/flCKYLQLz5\n\
MRm53XlT1mm+H9hjxBIdZSyj5KfD/ITyisfZY9jaNEZaObuz0ATpzAfdMlaFVLZn\n\
0NR7Vl5xbl6r5NIZjx7EQC7SaOp6l0Vn/5rU9TXvAoGAEiF3I1lo2ZXZGJl1on/b\n\
iPpls1mV3WwZUM45pEM2vEtdWAuBuacqAvzRIo00usp0FgOnn8Qw0Vn34w/6QK6n\n\
fd6Xt4J93RRBgf50oqTOEJ721lSzC0oiHaKnsx6x392c+FAwiWEe44SkfP77OC8R\n\
Xx0A2TVrAlJNKCpoB3soePw=\n\
-----END PRIVATE KEY-----\n\
";
#endif
#endif

static void messageArrived(MessageData *data)
{
	mqtt_exam_log("Message arrived on topic %.*s: %.*s", data->topicName->lenstring.len, (char *)data->topicName->lenstring.data, data->message->payloadlen, (char *)data->message->payload);

	// update_fota();
	// ThreadStop(&client.thread);
}

static void MQTTEchoTask(void *argv)
{
	/* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly for 3 times */
	Network network = {0};
	unsigned char sendbuf[80] = {0}, readbuf[80] = {0};
	int rc = 0,
		count = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
	SSLConfig SSLConfig = {
		.en = SSL_ENABLE,
#if SSL_ENABLE
		.profileIdx = PROFILE_IDX,
		.serverName = SERVER_DOMAIN,
		.serverPort = SERVER_PORT,
		.protocol = 0,
		.dbgLevel = 0,
		.sessionReuseEn = 0,
		.vsn = SSL_VSN_ALL,
		.verify = SSL_VERIFY_MODE,
		.cert.from = SSL_CERT_FROM_BUF,
#if (SERVER_PORT == 8307)
		.cert.path.rootCA = rootCA_path,
		.cert.path.clientKey = NULL,
		.cert.path.clientCert = NULL,
#elif (SERVER_PORT == 8883)
		.cert.path.rootCA = rootCA_path,
		.cert.path.clientKey = clientKey_path,
		.cert.path.clientCert = clientCert_path,
		.cert.clientKeyPwd.data = NULL,
		.cert.clientKeyPwd.len = 0,
#endif
		.cipherList = SSL_CIPHER_LIST,
		.CTRDRBGSeed.data = NULL,
		.CTRDRBGSeed.len = 0
#endif
	};

	argv = 0;
	NetworkInit(&network, &SSLConfig, PROFILE_IDX);
	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	char *address = SERVER_DOMAIN;
	if ((rc = NetworkConnect(&network, address, SERVER_PORT)) != 0)
	{
		mqtt_exam_log("Return code from network connect is %d", rc);
		MQTTClientDeinit(&client);
		goto exit;
	}

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = "4123444rrr333";

	connectData.username.cstring = "fota";
	connectData.password.cstring = "ThangTran@123@321#";

	if ((rc = MQTTConnect(&client, &connectData)) != 0)
	{
		mqtt_exam_log("Return code from MQTT connect is %d", rc);
		NetworkDisconnect(&network);
		MQTTClientDeinit(&client);
		goto exit;
	}
	else
		mqtt_exam_log("MQTT Connected");

#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&client)) != 0)
	{
		mqtt_exam_log("Return code from start tasks is %d", rc);
		NetworkDisconnect(&network);
		MQTTClientDeinit(&client);
		goto exit;
	}
#endif

	if ((rc = MQTTSubscribe(&client, "EC800M_REMOTE", QOS0, messageArrived)) != 0)
	{
		mqtt_exam_log("Return code from MQTT subscribe is %d", rc);

		rc = MQTTDisconnect(&client);
		if (rc == SUCCESS)
			mqtt_exam_log("MQTT Disconnected by client");
		else
			mqtt_exam_log("MQTT Disconnected failed by client");

		NetworkDisconnect(&network);

		MQTTClientDeinit(&client);

		goto exit;
	}

	while (1)
	{
		// MQTTMessage message;
		// char payload[30];

		// message.qos = 1;
		// message.retained = 0;
		// message.payload = payload;
		// sprintf(payload, "message number %d", count);
		// count++;
		// message.payloadlen = strlen(payload);

		// 		if ((rc = MQTTPublish(&client, "EC800M_REC", &message)) != 0)
		// 			mqtt_exam_log("Return code from MQTT publish is %d", rc);
		// #if !defined(MQTT_TASK)
		// 		if ((rc = MQTTYield(&client, 1000)) != 0)
		// 			mqtt_exam_log("Return code from yield is %d", rc);
		// #endif
		
		//MQTTSubscribe(&client, "EC800M_REMOTE", 2, messageArrived);
			mqtt_exam_log(" Code: %d",client.isconnected);

		// if ((rc = MQTTSubscribe(&client, "EC800M_REMOTE", 2, messageArrived)) != 0)
		// {
		// 	mqtt_exam_log("Return code from MQTT subscribe is %d", rc);

		// 	rc = MQTTDisconnect(&client);
		// 	if (rc == SUCCESS)
		// 		mqtt_exam_log("MQTT Disconnected by client");
		// 	else
		// 		mqtt_exam_log("MQTT Disconnected failed by client");

		// 	NetworkDisconnect(&network);

		// 	MQTTClientDeinit(&client);

		// 	goto exit;
		// }
		// if ((rc = MQTTYield(&client, 1000)) != 0)
		// {
		// 	mqtt_exam_log("Error in MQTTYield, error code: %d", rc);
		// 	break;
		// }
		// MQTTSetMessageHandler(&client, "EC800M_REMOTE",messageArrived);
		ql_rtos_task_sleep_s(5); // Tạm dừng 30 giây giữa các lần xử lý
		//	MQTTYield(&client, 1000);
	}

	rc = MQTTDisconnect(&client);
	if (rc == SUCCESS)
		mqtt_exam_log("MQTT Disconnected by client");
	else
		mqtt_exam_log("MQTT Disconnected failed by client");

	NetworkDisconnect(&network);

	MQTTClientDeinit(&client);

exit:

	mqtt_exam_log("========== mqtt test end ==========");
	ql_rtos_task_delete(NULL);
}

void StartMQTTTask(void)
{
	ql_task_t task = NULL;
	ql_rtos_task_create(&task, 8 * 1024, 100, "mqtt_test", MQTTEchoTask, NULL);
}

static void ql_nw_status_callback(int profile_idx, int nw_status)
{
	mqtt_exam_log("profile(%d) status: %d", profile_idx, nw_status);
}

int datacall_start(void)
{
	struct ql_data_call_info info = {0};
	char ip4_addr_str[16] = {0};

	mqtt_exam_log("wait for network register done");

	if (ql_network_register_wait(120) != 0)
	{
		mqtt_exam_log("*** network register fail ***");
		return -1;
	}
	else
	{
		mqtt_exam_log("doing network activating ...");

		ql_wan_start(ql_nw_status_callback);
		ql_set_auto_connect(PROFILE_IDX, TRUE);
		if (ql_start_data_call(PROFILE_IDX, 0, "v-internet", NULL, NULL, 0) == 0)
		{
			ql_get_data_call_info(PROFILE_IDX, 0, &info);

			mqtt_exam_log("info.profile_idx: %d", info.profile_idx);
			mqtt_exam_log("info.ip_version: %d", info.ip_version);
			mqtt_exam_log("info.v4.state: %d", info.v4.state);
			mqtt_exam_log("info.v4.reconnect: %d", info.v4.reconnect);

			inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
			mqtt_exam_log("info.v4.addr.ip: %s", ip4_addr_str);

			inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
			mqtt_exam_log("info.v4.addr.pri_dns: %s", ip4_addr_str);

			inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
			mqtt_exam_log("info.v4.addr.sec_dns: %s", ip4_addr_str);

			return 0;
		}

		mqtt_exam_log("*** network activated fail ***");
		return -1;
	}
}

static void MQTTTest(void *argv)
{
	(void)argv;

	mqtt_exam_log("========== mqtt test satrt ==========");

	if (datacall_start() == 0)
		StartMQTTTask();
	else
		mqtt_exam_log("========== mqtt test end ==========");
}

// application_init(MQTTTest, "mqtttest", 2, 14);

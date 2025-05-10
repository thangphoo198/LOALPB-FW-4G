#include <stdio.h>
#include <string.h>
#include "mp3.h"
#include "name_mp3.h"
#include "uart_usb.h"
#define DISK_FS "U:"
char ng_gui[30];
int number;
void PlayMP3File(const char *audioFileName)
{
    int ret;
    char filePath[32];

    memset(filePath, 0, sizeof(filePath));
    sprintf(filePath, "%s/%s", DISK_FS, audioFileName);
    // sprintf(filePath,"/ext/%s",audioFileName);
    usb_log("MP3 Path is %s\r\n", filePath);
    Play_mp3(filePath);
}

void removeWord(char *str, const char *word)
{
    char *pos;
    int len = strlen(word);

    // Kiểm tra xem từ có xuất hiện trong chuỗi không
    if (strstr(str, word) != NULL)
    {
        while ((pos = strstr(str, word)) != NULL)
        {
            memmove(pos, pos + len, strlen(pos + len) + 1);
        }
    }
    else
    {
        // printf("Từ '%s' không được tìm thấy trong chuỗi.\n", word);
    }
}
void NumberToWords(int number, char *result, int kieu)
{
    char *units[] = {"khong", "mot", "hai", "ba", "bon", "nam", "sau", "bay", "tam", "chin"};
    char *positions[] = {"", "nghin", "trieu", "ti"};
    // char duoi[16]={0};

    int parts[4]; // Tối đa 4 phần: tỷ, triệu, nghìn, đơn vị
    int numParts = 0;
    // tachchuso(number,8);
    while (number > 0)
    {
        parts[numParts++] = number % 1000;
        number /= 1000;
    }

    for (int i = numParts - 1; i >= 0; i--)
    {
        int part = parts[i];
        int hundreds = part / 100;
        int tens = (part % 100) / 10;
        int unitsPart = part % 10;
        // printf("tens:%d\r\n",tens);

        if (part > 0)
        {
            if (hundreds > 0)
            {
                strcat(result, units[hundreds]);
                strcat(result, " tram ");
            }
            else if (i < numParts - 1 && part > 0)
            {
                strcat(result, "khongtram ");
            }

            if (tens > 1)
            {
                strcat(result, units[tens]);
                if (kieu == 0)
                    strcat(result, " muoi2 ");
                else if (kieu == 1)
                {
                    if (unitsPart != 0)
                        strcat(result, " "); // bo muoi nếu đọc pin strcat(result, " muoi2 ");
                    else
                        strcat(result, " muoi2 ");
                }
                if (unitsPart == 1)
                {
                    strcat(result, "mot2 ");
                }
                else if (unitsPart == 5)
                {
                    strcat(result, "lam ");
                }
                else if (unitsPart > 0)
                {
                    strcat(result, units[unitsPart]);
                    strcat(result, " ");
                }
            }
            else if (tens == 1)
            { // so dau
                strcat(result, "muoi ");
                // else
                if (unitsPart == 1)
                {
                    strcat(result, "mot ");
                }
                else if (unitsPart == 5)
                {
                    strcat(result, "lam ");
                }
                else if (unitsPart > 0)
                {
                    strcat(result, units[unitsPart]);
                    strcat(result, " ");
                }
            }
            else if (tens == 0)
            {
                int chuc = part % 10;
                int dv = chuc / 10;
                // printf(" unitp: %d hundreds:%d part:%d chuc:%d dv:%d\r\n",unitsPart,hundreds,part,chuc,dv);
                if (i < numParts - 1 && unitsPart > 0 || (dv == 0 && chuc != 0))
                {
                    strcat(result, "le ");
                }
                if (unitsPart > 0)
                {
                    strcat(result, units[unitsPart]);
                    strcat(result, " ");
                }
            }

            strcat(result, positions[i]);
            // memcmp(duoi,positions[i],sizeof(positions[i]));
            //		    strncpy(duoi, positions[i], sizeof(duoi));
            //
            //		    duoi[sizeof(duoi) - 1] = '\0'; // Đảm bảo chuỗi được kết thúc bằng ký tự null
            //		    usb_log("duoi:%s\n",duoi);
            strcat(result, " ");
        }
    }
    // Remove the extra space at the end
    if (kieu == 0)
    {
        int len = strlen(result);
        if (result[len - 1] == ' ')
        {
            result[len - 1] = '\0';
        }
        strcat(result, "dong");
    }
    else
    {
        return;
    }
    //    if(kieu==0)
    //    {
    //    	if(strcmp(duoi,"nghin")==0)
    //    	{
    //    		strcat(result, "nghindong");
    //    	}
    //    	else if(strcmp(duoi,"trieu")==0)
    //    	{
    //    		strcat(result, "trieudong");
    //    	}
    //    	else
    //    	{
    //    		strcat(result, "dong");
    //    	}
    //
    //    	//strcat(result, "dong");
    //    }
    //    else if(type==1)

    usb_log("\nket qua:%s\n", result);
}

// Hàm phát âm thanh cho từng từ trong chuỗi chữ
void PlayWords(const char *words)
{
    // Tạo một bản sao của chuỗi đầu vào
    char *wordsCopy = malloc(strlen(words) + 1);
    if (wordsCopy == NULL)
    {
        usb_log("Memory allocation failed\n");
        return;
    }
    strcpy(wordsCopy, words);

    char *word = strtok(wordsCopy, " ");
    while (word != NULL)
    {
        char mp3File[32];
        sprintf(mp3File, "%s.mp3", word);
        PlayMP3File(mp3File);
        word = strtok(NULL, " ");
    }

    // Giải phóng bộ nhớ sau khi sử dụng
    free(wordsCopy);
}
void play_tien(char *str)
{
    // Chuỗi đầu vào
    // char str[] = "VietinBank:12/05/2023 16:33|TK:108870109999|GD:+35,900,000VND|SDC:35,500,224VND|ND:CT DEN:3132094444 Tuyet chuyen FT23132568760089; tai Napas";

    // Tìm vị trí của từ khóa "GD:+"
    char *pos = strstr(str, "+"); //+
    if (pos != NULL)
    {
        // Trích xuất số từ chuỗi sau "GD:+"
        char numStr[20];
        sscanf(pos, "+%[^VND]", numStr);

        // Xóa dấu phẩy khỏi chuỗi số
        char cleanNumStr[20];
        int j = 0;
        for (int i = 0; i < strlen(numStr); i++)
        {

            if (strcmp(ng_gui, "\"VTMONEY\"") == 0)
            {
                if (numStr[i] != '.')
                {
                    cleanNumStr[j++] = numStr[i];
                }
            }
            else
            {
                if (numStr[i] != ',')
                {
                    cleanNumStr[j++] = numStr[i];
                }
            }
        }
        cleanNumStr[j] = '\0';

        // Chuyển chuỗi số thành số nguyên
        number = atoi(cleanNumStr);

        // Chuyển đổi số thành chữ
        char result[256] = "";
        NumberToWords(number, result, 0);
        usb_log(" buff:%s \n tien: %s\n VND:%d \n", str, result, number);

        // Phát các file mp3 tương ứng với từng từ trong số tiền
        if (result[0] == 'l' && result[1] == 'e')
        {
            removeWord(result, "le ");
        }

        PlayMP3File("ting.mp3");
        PlayWords(result);
    }
    else
    {
        usb_log("Không tìm thấy từ khóa 'GD:+'\n");
    }

    memset(ng_gui, 0, sizeof(ng_gui));
    //    return 0;
}
void play_tien_mqtt(int t)
{

    char result[256] = "";
    NumberToWords(t, result, 0);
    usb_log("\n tien: %s\n VND:%d \n", result, number);

    // Phát các file mp3 tương ứng với từng từ trong số tiền
    if (result[0] == 'l' && result[1] == 'e')
    {
        removeWord(result, "le ");
    }

    PlayMP3File("ting.mp3");
    PlayWords(result);
}

void re_play(int num)
{
    if (num > 0)
    {
        // Chuyển đổi số thành chữ
        char result[256] = "";
        NumberToWords(num, result, 0);
        usb_log("tien: %s\n VND:%d \n", result, num);
        // Phát các file mp3 tương ứng với từng từ trong số tiền
        if (result[0] == 'l' && result[1] == 'e')
        {
            removeWord(result, "le ");
        }
        PlayWords(result);
    }
    else
    {
        usb_log("so tien khong hop le \n");
    }
}
void play_pin(int pin)
{
    if (pin == 0)
    {
        PlayMP3File(pinduoi10);
    }
    else
    {
        PlayMP3File(pincon);
        char result[32] = "";
        memset(result, 0, sizeof(result));
        NumberToWords(pin, result, 1);
        printf("\r\n%s\r\n", result);
        PlayWords(result);
        PlayMP3File(phantram);
    }
}



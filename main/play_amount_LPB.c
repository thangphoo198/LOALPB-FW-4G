#include "name_mp3.h"
const char *levels[] = { "", "nghin", "trieu", "ti", "nghin ti", "trieu ti", "ti ti" };
// Struct ánh xạ số với file âm thanh
typedef struct {
    int number;
    const char *filename;
} AudioMapEntry;
 
// Bảng ánh xạ số → file âm thanh
AudioMapEntry audioMap[] = {
    {0, khong}, {1, mot}, {2, hai}, {3, ba}, {4, bon},
    {5, nam}, {6, sau}, {7, bay}, {8, tam}, {9, chin},
    {10, muoi}, {20, haimuoi}, {30, bamuoi}, {40, bonmuoi},
    {50, nammuoi}, {60, saumuoi}, {70, baymuoi}, {80, tammuoi}, {90, chinmuoi}
};
 
// Lấy tên file âm thanh theo số
const char* getAudioFile(int number) {
    int mapSize = sizeof(audioMap) / sizeof(AudioMapEntry);
    for (int i = 0; i < mapSize; ++i) {
        if (audioMap[i].number == number)
            return audioMap[i].filename;
    }
 
    return "";
}
 
// Đọc số thành từng phần âm thanh
void sayNumber(int number, int isUnit) {    
    if (number < 10) {
        if (number == 1 && isUnit)
            PlayMP3File(mot);// "một"
        else
            PlayMP3File(getAudioFile(number));
    } else if (number < 20) {
        PlayMP3File(muoi);// "mười"
        int unit = number % 10;
        if (unit == 5)
            PlayMP3File(lam);// "lăm"
        else if (unit != 0)
            sayNumber(unit, 1);
    } else {
        int chuc = number / 10;
        int donvi = number % 10;
        PlayMP3File(getAudioFile(chuc * 10));
        if (donvi == 1)
            PlayMP3File(mot2);// "mốt"
        else if (donvi == 5)
            PlayMP3File(lam);// "lăm"
        else if (donvi != 0)
            sayNumber(donvi, 1);
    }
}
 
// Đọc 3 chữ số kèm đơn vị (nghìn, triệu, tỷ...)
// void sayGroup(int number, const char *unit, int isFirstGroup) {
//     int num = number / 100;
//     int chuc_donvi = number % 100;
 
//     if (num > 0) {
//         sayNumber(num, 0);
//         PlayMP3File(tram);// "trăm"
//     } else if (!isFirstGroup && chuc_donvi > 0) {
//         PlayMP3File(khong);// "không"
//     }
 
//     if (chuc_donvi > 0) {
//         if (chuc_donvi < 10 && num > 0)
//             PlayMP3File(le);// "lẻ"
//         sayNumber(chuc_donvi, 0);
//     }
 
//     if (number > 0 && strlen(unit) > 0)
//         PlayMP3File(unit);
// }
void sayGroup(int number, const char *unit, int isFirstGroup) {
    int num = number / 100;
    int chuc_donvi = number % 100;
 
    if (num > 0) {
        sayNumber(num, 0);
        PlayMP3File(tram);// "trăm"
    } else if (!isFirstGroup && chuc_donvi > 0) {
        PlayMP3File(khong);// "không"
        PlayMP3File(tram);// "trăm"
    }
 
    if (chuc_donvi > 0) {
        if (chuc_donvi < 10 && num > 0)
            PlayMP3File(le);// "lẻ"
        sayNumber(chuc_donvi, 0);
    }
 
    if (number > 0 && strlen(unit) > 0)
        PlayMP3File(unit);
}
 
// Đọc tổng số tiền
void readMoney(unsigned long long amount) {
    //PlayMP3File(ting);// "đồng"
    const char *units[] = {
        "", nghin, trieu, ti,
        nghinty, trieuty
    };
    int parts[10];
    int count = 0;

    if (amount == 0) {
        PlayMP3File(khong);// "không"
        PlayMP3File(dong);// "đồng"
        return;
    }
 
    while (amount > 0) {
        parts[count++] = amount % 1000;
        amount /= 1000;
    }
 
    for (int i = count - 1; i >= 0; --i) {
        // tránh đọc phần "000 nghìn tỷ" hoặc "000 triệu tỷ"
        if (parts[i] != 0 || i == 0)
            sayGroup(parts[i], units[i], i == count - 1);
    }
 
    PlayMP3File(dong);// "đồng"
    return;
}
 
// Hàm main
// int main() {
//     long long money = 1205015; // Một triệu hai trăm lẻ năm nghìn mười lăm
//     readMoney(money);
//     return 0;
// }
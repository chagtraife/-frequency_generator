/*
 * frequency_generator.cpp
 *
 * Created: 6/26/2023 8:14:20 PM
 * Author : ThangNguyen
 */ 

#include <avr/io.h>
#include <util/delay.h> 
#include <avr/interrupt.h>
//#include <math.h>

#define isDownPress  ((PINC & (1<<PINC5)) == 0)
#define isUpPress    ((PINC & (1<<PINC4)) == 0)
#define isRestPress  ((PINB & (1<<PINB0)) == 0)
#define RANGE_1      ((PINB & (1<<PINB3)) == 0)
#define RANGE_2      ((PINB & (1<<PINB4)) == 0)

#define PWM_POUT    DDRB1
#define LED7A       DDRD4
#define LED7B       DDRD5
#define LED7C       DDRD6
#define LED7D       DDRD1
#define LED7E       DDRD0
#define LED7F       DDRD3
#define LED7G       DDRD2
#define LED7DP      DDRD7
#define LED7_1      DDRC3
#define LED7_2      DDRC2
#define LED7_3      DDRC1
#define LED7_4      DDRC0

//magic number define
#define TIMER0_PRESCALER      (1 << CS01) | (1 << CS00) // clkI/O/64 (From prescaler)

// global variable define
unsigned int maxRange = 1000;
unsigned int minRange = 100;
unsigned int freq = 100;
unsigned int factor = 1;
uint8_t digitVal_1, digitVal_2, digitVal_3, digitVal_4 = 0;
uint8_t pointIdx = 0;
uint8_t digitIdx = 1; // range in 1-4
/*
    7 6 5 4 3 2 1 0
    DP C B A F G D E
0   1 0 0 0 0 1 0 0  -> 0x84
1	1 0 0 1 1 1 1 1  -> 0x9F
2   1 1 0 0 1 0 0 0  -> 0xC8
3   1 0 0 0 1 0 0 1  -> 0x89
4   1 0 0 1 0 0 1 1  -> 0x93
5   1 0 1 0 0 0 0 1  -> 0xA1
6   1 0 1 0 0 0 0 0  -> 0xA0
7   1 0 0 0 1 1 1 1  -> 0x8F
8   1 0 0 0 0 0 0 0  -> 0x80
9   1 0 0 0 0 0 0 1  -> 0x81
*/
uint8_t led7[10] = { 0x84, 0x9F, 0xC8, 0x89, 0x93, 0xA1, 0xA0, 0x8F, 0x80, 0x81 };
	
//uint8_t f_sin[1000] = {97, 97, 98, 98, 99, 99, 100, 100, 101, 101, 102, 102, 103, 103, 104, 104, 105, 105, 106, 106, 107, 107, 108, 108, 109, 109, 110, 110, 111, 111, 111, 112, 112, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118, 119, 119, 120, 120, 121, 121, 122, 122, 123, 123, 124, 124, 125, 125, 125, 126, 126, 127, 127, 128, 128, 129, 129, 130, 130, 131, 131, 131, 132, 132, 133, 133, 134, 134, 135, 135, 135, 136, 136, 137, 137, 138, 138, 139, 139, 139, 140, 140, 141, 141, 141, 142, 142, 143, 143, 144, 144, 144, 145, 145, 146, 146, 146, 147, 147, 147, 148, 148, 149, 149, 149, 150, 150, 151, 151, 151, 152, 152, 152, 153, 153, 153, 154, 154, 154, 155, 155, 156, 156, 156, 157, 157, 157, 157, 158, 158, 158, 159, 159, 159, 160, 160, 160, 161, 161, 161, 162, 162, 162, 162, 163, 163, 163, 164, 164, 164, 164, 165, 165, 165, 165, 166, 166, 166, 166, 167, 167, 167, 167, 168, 168, 168, 168, 168, 169, 169, 169, 169, 170, 170, 170, 170, 170, 171, 171, 171, 171, 171, 171, 172, 172, 172, 172, 172, 172, 173, 173, 173, 173, 173, 173, 173, 174, 174, 174, 174, 174, 174, 174, 174, 175, 175, 175, 175, 175, 175, 175, 175, 175, 175, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 177, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 175, 175, 175, 175, 175, 175, 175, 175, 175, 175, 174, 174, 174, 174, 174, 174, 174, 174, 173, 173, 173, 173, 173, 173, 173, 172, 172, 172, 172, 172, 172, 171, 171, 171, 171, 171, 171, 170, 170, 170, 170, 170, 169, 169, 169, 169, 168, 168, 168, 168, 168, 167, 167, 167, 167, 166, 166, 166, 166, 165, 165, 165, 165, 164, 164, 164, 164, 163, 163, 163, 162, 162, 162, 162, 161, 161, 161, 160, 160, 160, 159, 159, 159, 158, 158, 158, 157, 157, 157, 157, 156, 156, 156, 155, 155, 154, 154, 154, 153, 153, 153, 152, 152, 152, 151, 151, 151, 150, 150, 149, 149, 149, 148, 148, 147, 147, 147, 146, 146, 146, 145, 145, 144, 144, 144, 143, 143, 142, 142, 141, 141, 141, 140, 140, 139, 139, 139, 138, 138, 137, 137, 136, 136, 135, 135, 135, 134, 134, 133, 133, 132, 132, 131, 131, 131, 130, 130, 129, 129, 128, 128, 127, 127, 126, 126, 125, 125, 125, 124, 124, 123, 123, 122, 122, 121, 121, 120, 120, 119, 119, 118, 118, 117, 117, 116, 116, 115, 115, 114, 114, 113, 113, 112, 112, 111, 111, 111, 110, 110, 109, 109, 108, 108, 107, 107, 106, 106, 105, 105, 104, 104, 103, 103, 102, 102, 101, 101, 100, 100, 99, 99, 98, 98, 97, 97, 96, 95, 95, 94, 94, 93, 93, 92, 92, 91, 91, 90, 90, 89, 89, 88, 88, 87, 87, 86, 86, 85, 85, 84, 84, 83, 83, 82, 82, 82, 81, 81, 80, 80, 79, 79, 78, 78, 77, 77, 76, 76, 75, 75, 74, 74, 73, 73, 72, 72, 71, 71, 70, 70, 69, 69, 68, 68, 68, 67, 67, 66, 66, 65, 65, 64, 64, 63, 63, 62, 62, 62, 61, 61, 60, 60, 59, 59, 58, 58, 58, 57, 57, 56, 56, 55, 55, 54, 54, 54, 53, 53, 52, 52, 52, 51, 51, 50, 50, 49, 49, 49, 48, 48, 47, 47, 47, 46, 46, 46, 45, 45, 44, 44, 44, 43, 43, 42, 42, 42, 41, 41, 41, 40, 40, 40, 39, 39, 39, 38, 38, 37, 37, 37, 36, 36, 36, 36, 35, 35, 35, 34, 34, 34, 33, 33, 33, 32, 32, 32, 31, 31, 31, 31, 30, 30, 30, 29, 29, 29, 29, 28, 28, 28, 28, 27, 27, 27, 27, 26, 26, 26, 26, 25, 25, 25, 25, 25, 24, 24, 24, 24, 23, 23, 23, 23, 23, 22, 22, 22, 22, 22, 22, 21, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 24, 24, 24, 24, 25, 25, 25, 25, 25, 26, 26, 26, 26, 27, 27, 27, 27, 28, 28, 28, 28, 29, 29, 29, 29, 30, 30, 30, 31, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 36, 37, 37, 37, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43, 44, 44, 44, 45, 45, 46, 46, 46, 47, 47, 47, 48, 48, 49, 49, 49, 50, 50, 51, 51, 52, 52, 52, 53, 53, 54, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58, 58, 59, 59, 60, 60, 61, 61, 62, 62, 62, 63, 63, 64, 64, 65, 65, 66, 66, 67, 67, 68, 68, 68, 69, 69, 70, 70, 71, 71, 72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 79, 80, 80, 81, 81, 82, 82, 82, 83, 83, 84, 84, 85, 85, 86, 86, 87, 87, 88, 88, 89, 89, 90, 90, 91, 91, 92, 92, 93, 93, 94, 94, 95, 95, 96};
//uint8_t f_sin[500] = {97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 131, 132, 133, 134, 135, 136, 137, 138, 139, 139, 140, 141, 142, 143, 144, 144, 145, 146, 147, 147, 148, 149, 150, 151, 151, 152, 153, 153, 154, 155, 156, 156, 157, 157, 158, 159, 159, 160, 161, 161, 162, 162, 163, 164, 164, 165, 165, 166, 166, 167, 167, 168, 168, 168, 169, 169, 170, 170, 171, 171, 171, 172, 172, 172, 173, 173, 173, 173, 174, 174, 174, 174, 175, 175, 175, 175, 175, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 177, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 175, 175, 175, 175, 175, 174, 174, 174, 174, 173, 173, 173, 173, 172, 172, 172, 171, 171, 171, 170, 170, 169, 169, 168, 168, 168, 167, 167, 166, 166, 165, 165, 164, 164, 163, 162, 162, 161, 161, 160, 159, 159, 158, 157, 157, 156, 156, 155, 154, 153, 153, 152, 151, 151, 150, 149, 148, 147, 147, 146, 145, 144, 144, 143, 142, 141, 140, 139, 139, 138, 137, 136, 135, 134, 133, 132, 131, 131, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 62, 61, 60, 59, 58, 57, 56, 55, 54, 54, 53, 52, 51, 50, 49, 49, 48, 47, 46, 46, 45, 44, 43, 42, 42, 41, 40, 40, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 25, 24, 24, 23, 23, 22, 22, 22, 21, 21, 21, 20, 20, 20, 20, 19, 19, 19, 19, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 24, 24, 25, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 31, 31, 32, 32, 33, 34, 34, 35, 36, 36, 37, 37, 38, 39, 40, 40, 41, 42, 42, 43, 44, 45, 46, 46, 47, 48, 49, 49, 50, 51, 52, 53, 54, 54, 55, 56, 57, 58, 59, 60, 61, 62, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95};
uint8_t f_sin[200] = {97, 99, 102, 104, 107, 109, 111, 114, 116, 119, 121, 124, 126, 128, 131, 133, 135, 137, 139, 141, 144, 146, 147, 149, 151, 153, 155, 157, 158, 160, 161, 163, 164, 165, 167, 168, 169, 170, 171, 172, 173, 173, 174, 175, 175, 176, 176, 176, 176, 176, 177, 176, 176, 176, 176, 176, 175, 175, 174, 173, 173, 172, 171, 170, 169, 168, 167, 165, 164, 163, 161, 160, 158, 157, 155, 153, 151, 149, 147, 146, 144, 141, 139, 137, 135, 133, 131, 128, 126, 124, 121, 119, 116, 114, 111, 109, 107, 104, 102, 99, 97, 94, 91, 89, 86, 84, 82, 79, 77, 74, 72, 69, 67, 65, 62, 60, 58, 56, 54, 52, 49, 47, 46, 44, 42, 40, 38, 36, 35, 33, 32, 30, 29, 28, 26, 25, 24, 23, 22, 21, 20, 20, 19, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 19, 20, 20, 21, 22, 23, 24, 25, 26, 28, 29, 30, 32, 33, 35, 36, 38, 40, 42, 44, 46, 47, 49, 52, 54, 56, 58, 60, 62, 65, 67, 69, 72, 74, 77, 79, 82, 84, 86, 89, 91, 94}; 
//uint8_t f_sin[300] = {97, 98, 100, 102, 103, 105, 107, 108, 110, 111, 113, 115, 116, 118, 120, 121, 123, 124, 126, 128, 129, 131, 132, 134, 135, 137, 138, 139, 141, 142, 144, 145, 146, 147, 149, 150, 151, 152, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 167, 168, 169, 170, 170, 171, 171, 172, 173, 173, 174, 174, 174, 175, 175, 175, 176, 176, 176, 176, 176, 176, 176, 177, 176, 176, 176, 176, 176, 176, 176, 175, 175, 175, 174, 174, 174, 173, 173, 172, 171, 171, 170, 170, 169, 168, 167, 167, 166, 165, 164, 163, 162, 161, 160, 159, 158, 157, 156, 155, 154, 152, 151, 150, 149, 147, 146, 145, 144, 142, 141, 139, 138, 137, 135, 134, 132, 131, 129, 128, 126, 124, 123, 121, 120, 118, 116, 115, 113, 111, 110, 108, 107, 105, 103, 102, 100, 98, 97, 95, 93, 91, 90, 88, 86, 85, 83, 82, 80, 78, 77, 75, 73, 72, 70, 69, 67, 65, 64, 62, 61, 59, 58, 57, 55, 54, 52, 51, 49, 48, 47, 46, 44, 43, 42, 41, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 26, 25, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 21, 22, 22, 23, 23, 24, 25, 26, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 41, 42, 43, 44, 46, 47, 48, 49, 51, 52, 54, 55, 57, 58, 59, 61, 62, 64, 65, 67, 69, 70, 72, 73, 75, 77, 78, 80, 82, 83, 85, 86, 88, 90, 91, 93, 95};
//uint8_t f_sin[400] = {166, 167, 167, 168, 168, 169, 169, 170, 170, 171, 171, 172, 172, 173, 173, 173, 174, 174, 174, 175, 175, 175, 175, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 177, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 175, 175, 175, 175, 174, 174, 174, 173, 173, 173, 172, 172, 171, 171, 170, 170, 169, 169, 168, 168, 167, 167, 166, 165, 165, 164, 163, 163, 162, 161, 160, 160, 159, 158, 157, 157, 156, 155, 154, 153, 152, 151, 150, 149, 148, 147, 147, 146, 145, 144, 143, 141, 140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 129, 128, 127, 126, 125, 124, 122, 121, 120, 119, 118, 116, 115, 114, 113, 111, 110, 109, 108, 107, 105, 104, 103, 102, 100, 99, 98, 97, 95, 94, 93, 91, 90, 89, 88, 86, 85, 84, 83, 82, 80, 79, 78, 77, 75, 74, 73, 72, 71, 69, 68, 67, 66, 65, 64, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 50, 49, 48, 47, 46, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 36, 35, 34, 33, 33, 32, 31, 30, 30, 29, 28, 28, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21, 20, 20, 20, 19, 19, 19, 18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 19, 19, 19, 20, 20, 20, 21, 21, 22, 22, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 33, 33, 34, 35, 36, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 46, 47, 48, 49, 50, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 64, 65, 66, 67, 68, 69, 71, 72, 73, 74, 75, 77, 78, 79, 80, 82, 83, 84, 85, 86, 88, 89, 90, 91, 93, 94, 95};
//uint8_t f_sin[100] = {97, 102, 107, 111, 116, 121, 126, 131, 135, 139, 144, 147, 151, 155, 158, 161, 164, 167, 169, 171, 173, 174, 175, 176, 176, 177, 176, 176, 175, 174, 173, 171, 169, 167, 164, 161, 158, 155, 151, 147, 144, 139, 135, 131, 126, 121, 116, 111, 107, 102, 97, 91, 86, 82, 77, 72, 67, 62, 58, 54, 49, 46, 42, 38, 35, 32, 29, 26, 24, 22, 20, 19, 18, 17, 17, 17, 17, 17, 18, 19, 20, 22, 24, 26, 29, 32, 35, 38, 42, 46, 49, 54, 58, 62, 67, 72, 77, 82, 86, 91};
//uint8_t f_sin[250] = {97, 99, 101, 103, 105, 107, 109, 111, 112, 114, 116, 118, 120, 122, 124, 126, 128, 130, 131, 133, 135, 137, 139, 140, 142, 144, 145, 147, 148, 150, 151, 153, 154, 156, 157, 158, 159, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 171, 172, 173, 173, 174, 174, 175, 175, 175, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 176, 175, 175, 175, 174, 174, 173, 173, 172, 171, 171, 170, 169, 168, 167, 166, 165, 164, 163, 162, 161, 159, 158, 157, 156, 154, 153, 151, 150, 148, 147, 145, 144, 142, 140, 139, 137, 135, 133, 131, 130, 128, 126, 124, 122, 120, 118, 116, 114, 112, 111, 109, 107, 105, 103, 101, 99, 97, 94, 92, 90, 88, 86, 84, 82, 81, 79, 77, 75, 73, 71, 69, 67, 65, 63, 62, 60, 58, 56, 54, 53, 51, 49, 48, 46, 45, 43, 42, 40, 39, 37, 36, 35, 34, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 21, 22, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 34, 35, 36, 37, 39, 40, 42, 43, 45, 46, 48, 49, 51, 53, 54, 56, 58, 60, 62, 63, 65, 67, 69, 71, 73, 75, 77, 79, 81, 82, 84, 86, 88, 90, 92, 94};

uint16_t f_time[200] = {0};
uint8_t f_cycleId[200] = {0};
uint8_t N = 0;
uint8_t idx = 0;

uint32_t K = 100;
uint32_t timeIdx = 0;

uint16_t cnt_cycle = 0;
uint16_t time_scale = 0;
uint16_t factor_scale = 1;


void init_TC0(void);
void init_TC1(void);
void init_GPIO(void);
void readButton(void);
void display(void);

void setDisplay(unsigned int number, unsigned int f);
void increaseFreq(void);
void decreaseFreq(void);
void resetFreq(void);
void readRange(void);
void update(void);
void setPulse(unsigned int _freq, unsigned int _factor);

int main(void)
{
	init_GPIO();
	
	readRange();
	update();
		
	init_TC0(); //init timer1 for display
	init_TC1(); //init timer for pulse output
	    
	//enable interrupt
	sei();
	

    /* Replace with your application code */
    while (1) 
    {
		readButton();
		_delay_ms(100);
    }
}

void init_TC0(void)
{
	TIMSK |= (1 << TOIE0);        // interrupt enable - here overflow
	TCCR0 |= TIMER0_PRESCALER;    // use defined prescaler value
}

void init_TC1(void)
{
	// this code sets up counter1 for an 39,8kHz, Fast PWM
	// @ 8Mhz Clock

	TIMSK |= (1 << TOIE1);
	ICR1 = 0x00C8;
	OCR1A = 0x000C;
		
	TCCR1A |= (1 << COM1A1);    // set none-inverting mode
	
	//Mode 14: Fast PWM
	// TOP: ICR1
	// PWM_fequency = clock_speed / [Prescaller_value * (1 + TOP_Value) ]
	TCCR1A |= (1 << WGM11);    
	TCCR1B |= (1 << WGM13) | (1 << WGM12);
		
	TCCR1B |= (1 << CS10); // No Prescaling
}

void init_GPIO(void)
{
	//set RB1 as output
	DDRB |= (1 << DDRB1);	
	//set RC0, RC1, RC2, RC3 as output
	DDRC |= 0x0F;
	// set RDx as output
	DDRD = 0xFF;
	
	//set PC4, PC5 as input
	DDRC &= ~((1 << DDRC5) | (1 << DDRC4));
	PORTC |= (1 << PORTC5) | (1 << PORTC4);
}

void readButton(void)
{
	readRange();
	if (isDownPress) {
		_delay_ms(50);
		int cnt = 0;
		bool isLongPress = false;
		while (isDownPress && (cnt < 25)) {
			cnt++;
			if (cnt == 25){
				isLongPress = true;
			}
			_delay_ms(50);
		}
		
		if (isLongPress){
			cnt = 0;
			while(isDownPress) {
				cnt++;
				if (cnt % 4 == 0) {
					decreaseFreq();
				}
				_delay_ms(50);
			}
		} else if(!isDownPress) {
			decreaseFreq();
		}
	}
	
	if (isUpPress) {
		_delay_ms(50);
		int cnt = 0;
		bool isLongPress = false;
		while (isUpPress && (cnt < 25)) {
			cnt++;
			if (cnt == 25){
				isLongPress = true;
			}
			_delay_ms(50);
		}
			
		if (isLongPress){
			cnt = 0;
			while(isUpPress) {
				cnt++;
				if (cnt % 4 == 0) {
					increaseFreq();
				}
				_delay_ms(50);
			}
			} else if(!isUpPress) {
			increaseFreq();
		}
	}
	
	if (isRestPress) {
		_delay_ms(50);
        if (isRestPress){
	       resetFreq();
		}
	}
}

void display(void)
{
	PORTC |= 0x0F;
	switch (digitIdx) {
		case 1: {
			PORTD = led7[digitVal_1];
			if (pointIdx == 1){
				PORTD &= ~(1<<PORTD7);
			}
			PORTC &= ~(1<<PORTC3);
			break;
		}
		case 2: {
			PORTD = led7[digitVal_2];
			if (pointIdx == 2){
				PORTD &= ~(1<<PORTD7);
			}
			PORTC &= ~(1<<PORTC2);
			break;
		}
		case 3: {
			PORTD = led7[digitVal_3];
			if (pointIdx == 3){
				PORTD &= ~(1<<PORTD7);
			}
			PORTC &= ~(1<<PORTC1);
			break;
		}
		case 4: {
			if (digitVal_4 == 10) break;
			PORTD = led7[digitVal_4];
			if (pointIdx == 4){
				PORTD &= ~(1<<PORTD7);
			}
			PORTC &= ~(1<<PORTC0);		
			break;
		}
	}
	
	if (digitIdx == 4) {
		digitIdx = 1;
	} else {
		digitIdx++;
	}
}

void setDisplay(unsigned int number, unsigned int f)
{
	digitVal_1 = number % 10;
	digitVal_2 = (number /10) % 10;
	digitVal_3 = (number /100) % 10;
	if ((number < 1000) && (f !=1000)){
		digitVal_4 = 10; // mean do not display
	} else{
		digitVal_4 = (number /1000) % 10;
	}
	
	if (f == 1){
		pointIdx = 0;
	} else if (f == 10) {
		pointIdx = 2;
	} else if (f == 100) {
		pointIdx = 3;
	} else if (f == 1000) {
		pointIdx = 4;
	}
}

void increaseFreq(void)
{
	if (freq < 1000) {
		freq += 10;
		update();
	}
}

void decreaseFreq(void)
{
	if (freq > 100) {
		freq -= 10;
		update();
	}
}

void resetFreq(void)
{
	freq = minRange;
	update();
}

void readRange(void)
{
	unsigned int newFactor;
	if (RANGE_1 && RANGE_2){
		newFactor = 1000;
	} else if (RANGE_1)
	{
		newFactor = 100;
	} else if (RANGE_2)
	{
		newFactor = 10;
	} else {
		newFactor = 1;
	}
	
	if (newFactor != factor) {
		unsigned int newFreq;
		if (newFactor > factor) {
			newFreq = freq * (newFactor / factor);
		} else {
			newFreq = freq * newFactor / factor;
		}
		
		if ((newFreq < minRange) || (maxRange < newFreq)) {
			newFreq = minRange;
		}
		freq = newFreq;
		factor = newFactor;
		update();
	}
}

void update(void)
{
	setDisplay(freq, factor);
	setPulse(freq, factor);
}

void setPulse(unsigned int _freq, unsigned int _factor)
{
	uint16_t _freq_tmp =  _freq/10;
	K = (800000 * _factor)  / (201 * _freq_tmp);
	
	if (K < 200) {
		N = K;
	} else {
		N = 200;
	}
	
	if (_factor == 1000) factor_scale = 10;
	else factor_scale = 1;
	
	for (uint8_t i = 0; i < N; i++) {
		f_time[i] = K * i / (N * factor_scale);
		f_cycleId[i] = uint16_t(i * 200) / N;
	}
}

ISR (TIMER0_OVF_vect)
{
	display();
}

ISR (TIMER1_OVF_vect)
{	
	timeIdx++;
	if (timeIdx >= K) {
		timeIdx = 0;
		idx = 0;
		cnt_cycle = 0;
		time_scale = 0;
		OCR1A = f_sin[0];
	}
	
	if (cnt_cycle == factor_scale) {
		cnt_cycle = 0;
		time_scale++;
		uint8_t idxNext = idx + 1;
		if (idxNext >= N) idxNext = N - 1;
		if (time_scale >= f_time[idxNext]) {
			idx = idxNext;
			uint8_t cycleIdx = f_cycleId[idx];
			uint8_t cycle = f_sin[cycleIdx];
			OCR1A = cycle;
		}
	}
	cnt_cycle++;
}


#include <8051.h>
#include "Keypad4x4.h"
#define TIMER_VAL 15536
#define TAIL 200

unsigned char loc = 0,note=0x10,index=0,cnt=0;
unsigned short base = 0x0000;//預設LED全為零
unsigned char Dig[4];
unsigned char save ,on = 0, record=0, play=0;

const unsigned char single_digit[16]={0b11000000,0b11111001,0b10100100,0b10110000,0b10011001,0b10010010,0b10000011,0b11111000,
								      0b10000000,0b10010000,0b10001000,0b10000011,0b11000110,0b10100001,0b10000110,0b10001110};//LED 0~F


const unsigned short tone[] ={ //音調table
	65536-1000000/(2*262),  //C  0
	65536-1000000/(2*277),  //C# 1 
	65536-1000000/(2*294),	//D  2
	65536-1000000/(2*311),  //D# 3
	65536-1000000/(2*330),  //E  4 
	65536-1000000/(2*349),  //F  5
	65536-1000000/(2*370),  //F# 6
	65536-1000000/(2*392),  //G  7 
	65536-1000000/(2*415),  //G# 8
	65536-1000000/(2*440),	//A  9
	65536-1000000/(2*466),  //A# 10
	65536-1000000/(2*494),  //B  11
	0                       //BLANK
};

__xdata  unsigned char store[TAIL]={0}; //存音調
__xdata  unsigned short beat[TAIL]={0}; //存節拍


void display(int dig, char num) {
	if (dig==0){ //顯示當前那位數七段，其餘關閉
		P1_1=0;
		P1_2=1;
		P1_3=1;
		P1_4=1;
	}	
	else if (dig==1){  //顯示當前那位數七段，其餘關閉
		P1_1=1;
		P1_2=0;
		P1_3=1;
		P1_4=1;
	}
	else if (dig==2){  //顯示當前那位數七段，其餘關閉
		P1_1=1;
		P1_2=1;
		P1_3=0;
		P1_4=1;
	}
	else if (dig==3){  //顯示當前那位數七段，其餘關閉
		P1_1=1;
		P1_2=1;
		P1_3=1;
		P1_4=0;
	}
	P2=single_digit[num];
}


int main() {

	P1_0=0;//關音樂
	TH1 = TIMER_VAL >> 8;//以下初始化
	TL1 = TIMER_VAL & 0xff;
	TH0 = tone[0] >> 8;
	TL0 = tone[0] & 0xff;
	TMOD = 0x11;
	TCON = 0x50;
	IE = 0x8a;
	
	while(1)
	{	
		Dig[0] = base >> 12;// Dig[0]到Dig[3] 分別為左一到左四
		Dig[1] = (base << 4) >> 12;
		Dig[2] = (base << 8) >> 12;
		Dig[3] = (base << 12) >> 12;
		for (unsigned char i = 0; i < 4; i++) {
			display(i, Dig[i]);//顯示四位顯示器
			for (unsigned char d1 = 0; d1 < 10; d1 ++) {      //延遲
				for (unsigned char d2 = 0; d2 < 8; d2 ++) {
				}
			}
		}
		note=number();
		if  (note <16) {//有偵測到存到save
			save = note;
			on = 1;
		}

		
		if	(note == 0x10 && on == 1){   //debounce 
			base = (base << 4) + save;
			if (save == 12){	         //按下C開始錄製
				record=1;				 //record 變1
				play=0;					
				loc=0;
				for (unsigned char j=0;j<TAIL;j++)   //每次錄製初始
					store[j]=12; 
			}
			else if (save == 13){		//按下D開始撥放
				record=0;				// record 變0
				play=1;					//  play  變1
				index=0;
			}	
			on=0;
		}
		
	}
}

void timer_isr1 (void) __interrupt (3) __using (3) {//timer1 每次interuupt 0.05秒 
	
	
	if (record == 1 ){
		if ( save < 12 && on==1){  
			store[loc]=save;    //存音調
		}
		else {			
			store[loc]=12;  //存空白
		}
		cnt++;
		beat[loc]=cnt;
		loc++;
		cnt=0;
	}
	
	if (play == 1){
		beat[index]--;
		if (beat[index]==0)  //當前的index值為0 , index+1
			index++;
	}
	
	if (index == loc){     // index 從0開始，當值等於loc ,play設0
		play=0;
	}
	
	TH1 = TIMER_VAL >> 8;    //reset timer1
	TL1 = TIMER_VAL & 0xff;  //reset timer1
}

void timer_isr (void) __interrupt (1) __using (1) {//timer0 音符頻率
	if (note < 12 && play==0){
		P1_0 = !P1_0;
		TH0 = tone[note] >> 8;//reset timer0
		TL0 = tone[note] & 0xff;//reset timer0
	}
	else if (play==1){    // 播放mode
		if (store[index]==12){   //若是空白
			P1_0=1;	             //關掉喇叭
		}
		else {
			P1_0 = !P1_0;        //其餘播放
			TH0 = tone[store[index]] >> 8;      //reset timer0
			TL0 = tone[store[index]] & 0xff;    //reset timer0
		}
	}
}
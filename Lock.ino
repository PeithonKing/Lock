#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Keypad.h>

/*
User Instructions:- 
1) Pressing A helps to enter into input mode for
	USERNAME and also acts as the "ENTER" key!
2) Presssing B helps it to lock if it is unlocked!
3) Username max length = 9
4) Password max length = 9
5) Passwords and Usernames are all numeric only
5) Maximum number of users supported = 50 (+1 admin)
6) Admin Username and Password both are "1234" initially.
	They have to be changed on first Login.
7) 
*/

// 	                *** DONT FORGET ***
// EEPROM space [0] stores if admin password has been
// changed or not. If no, value is 0, else value is 1.
// if(EEPROM.read(0) == 0){reset();}  // UNIMPORTANT LINE MAYBE...
///////////////////// If you see 0 at [0] remember to reset
// EEPROM space [1023] stores current number of users (including admin)
// unused EEPROM space = [1021] and [1022]
// 
// All the spaces except [0], [1023] and the unused ones, are char type
// So while reading them don't try to read them in int,
// to store them, only use char type variables
// [0], and [1023] have to be read in int type vaiable!

LiquidCrystal lcd1(0, 1, 10, 11, 12, 13);

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
char key;

void setup(){
	lcd1.begin(16,2);
}

bool ready = true;

void loop(){
	if(ready){Ready();}
	key = 'E';
	key = keypad.getKey();
	if(key != 'E'){
		if(ready){
			if(key == 'A'){
				ready = false;
				lcd1.clear();
				lcd1.setCursor(0, 0);
				lcd1.print("1: Open Gate");
				lcd1.setCursor(0, 1);
				lcd1.print("2: + / - Users");
				while(true){
					char res = 'E';
					while(res == 'E'){res = keypad.getKey();}
					if(res == '1'){
						bool lo = login();
						if(lo){gateman();}
						else{
							lcd1.clear();
							lcd1.print("Pehli Fursat");
							lcd1.setCursor(1,6);
							lcd1.print("Me Nikal");
							delay(1000);
							lcd1.clear();
						}
						break;
					}
					if(res == '2'){
						lcd1.clear();
						lcd1.setCursor(0, 0);
						lcd1.print("1: Add User");
						lcd1.setCursor(0, 1);
						lcd1.print("2: Remove User");					
						while(true){
							res = 'E';
							while(res == 'E'){res = keypad.getKey();}
							if(res == '1'){
								if(EEPROM.read(1023) == 1){AddUser();}
								else{
									lcd1.clear();
									lcd1.print("Memory Full!");
									lcd1.setCursor(0, 1);
									lcd1.print("No more new User");
									delay(1000);
									lcd1.clear();
								}
							}
							if(res == '2'){RemoveUser();}
						}
						break;
					}
				}
			}
		}
	}
}

void Ready(){
	lcd1.print("Aritra's Lock");
	lcd1.setCursor(8,1);
	lcd1.print("Ready...");
}

void AddUser(){
	lcd1.clear();
	lcd1.print("To Add User");
	lcd1.setCursor(3, 1);
	lcd1.print("Login First!");
	delay(2000);
	bool lo = login();
	if(lo){
		int n = 0;
		bool WeDo = false;
		while(n < 1010){
			n = n + 20;
			if(EEPROM.read(n) == ' '){
				WeDo = true;
				break;
			}
		}
		if(n == 1001){EEPROM.write(1023,0);}
		if(WeDo){
			int st = (20*n+21);
			while(true){
				String name = takeInput("Username: ");
				int is = matchName(name);
				if(is > 0){
					lcd1.print("Username Occupied!");
					lcd1.setCursor(0,1);
					lcd1.print("Try another !!");
					delay(1500);
					lcd1.clear();
				}
				else{
					char a;
					for(int a1 = 0; a1 <= name.length(); a1++){
						a = name[a1];
						EEPROM.write(st, a);
						st++;
					}
					st = (20*n+31);
					String pwd = takeInput("Password: ");
					for(int a2 = 0; a2 <= pwd.length(); a2++){
						a = pwd[a2];
						EEPROM.write(st, a);
						st++;
					}
					lcd1.print("New User Created");
					delay(750);
					lcd1.clear();
					break;
				}
			}
		}
	}
	else{
		lcd1.clear();
		lcd1.print("You cant do this");
		lcd1.setCursor(1,0);
		lcd1.print("without login");
		delay(1000);
		lcd1.clear();
	}
}

void RemoveUser(){
	lcd1.clear();
	lcd1.print("To Remove User,");
	lcd1.setCursor(3, 1);
	lcd1.print("Login first!");
	delay(2000);
	bool lo = login();
	if(lo){
		String name = takeInput("Name to remove:");
		int a = matchName(name);
		if(a > 0){
			String pwd = takeInput("Password:");
			if(matchPwd(pwd, a)){
				a = a - 10;
				for(int d = 0; d < 20; d++){
					EEPROM.update(a, ' ');
					a++;
				}
				lcd1.clear();
				lcd1.print("Account Deleted");
				lcd1.setCursor(0, 1);
				lcd1.print("Succesfully!");
				delay(1000);
				lcd1.clear();
			}
			else{
				lcd1.clear();
				lcd1.print("Account Deletion");
				lcd1.setCursor(8, 1);
				lcd1.print("Aborted!");
				delay(1000);
				lcd1.clear();
			}
		}
}

bool login(){
	String name = takeInput("Username:");
	int a = matchName(name);
	if(a > 0){
		String pwd = takeInput("Password:");
		return matchPwd(pwd, a);
	}
}

String takeInput(String ask){
	String got = "";
	lcd1.clear();
	lcd1.print(ask);
	lcd1.setCursor(0, 1);
	lcd1.cursor();
	lcd1.blink();
	char res2;
	while(true){
		res2 = keypad.getKey();
		if(res2){
			if(res2 == 'A'){break;}
			//if(res2 == 'B'){;}            //tried to have a backspace option
			lcd1.print(res2);
			got.concat(res2);
		}
	}
	lcd1.noBlink();
	lcd1.noCursor();
	lcd1.clear();
	lcd1.print("You Entered: ");
	lcd1.setCursor(0, 1);
	lcd1.print(got);
	delay(1500);
	lcd1.clear();
	lcd1.print("Are you Sure?");
	lcd1.setCursor(0,1);
	lcd1.print("1: Yes    2: No");
	delay(1000);
	while(true){
		res2 = keypad.getKey();
		if(res2 == '1'){
			if(got.length()>9){
				lcd1.print("Maximum length");
				lcd1.setCursor(1,0);
				lcd1.print("should be 9");
				delay(1000);
				lcd1.clear();
				lcd1.print("  Try Again !!  ");
				delay(500);
				got = takeInput(ask);
			}
			lcd1.clear();
			lcd1.print("Returning Text");
			delay(1000);
			lcd1.clear();
			return got;
		}
		if(res2 == '2'){
			return takeInput(ask);
		}
	}
}

int matchName(String name){
	String comp;
	char k = 'E';
	int r = 1;
	int ret = 0;
	while(r < 1021){
		comp = ' ';
		while(k != ' '){
			k = EEPROM.read(r);
			comp.concat(k);
			r++;
		}
		if(name.equals(comp.trim())){
			ret = (r-(r%10)+11);
			return ret;
			break;
		}
		else{r = (r-(r%10)+21);}
	}
	if(ret == 0){return ret;}
}

bool matchPwd(String pwd, int r){
	String comp = ' ';
	char k = 'E';
	bool ret = false;
	while(k != ' '){
		k = EEPROM.read(r);
		comp.concat(k);
		r++;
	}
	if(pwd.equals(comp.trim())){ret = true;}
	else{return ret;}
}

void gateman(){
	lcd1.clear();
	lcd1.print("Gate Open!");
	lcd.setCursor(0,1);
	lcd1.print("B: Close Gate!");
	char keyL = "E";
	while(keyL != "B"){keypad.getKey();}
	lcd1.clear();
	lcd1.print("Closing Gate!");
	delay(500);
	clear();
}

bool WPwd(){
	lcd1.clear();
	lcd1.print("Invalid Password");
	lcd1.setCursor(1, 1);
	lcd1.print("Try Again Later");
	delay(2000);
	lcd1.clear();
	return false;
}

int WUsn(){
	lcd1.clear();
	lcd1.print("Invalid Username");
	lcd1.setCursor(1, 1);
	lcd1.print("Try Again Later");
	delay(2000);
	lcd1.clear();
	return false;
}

char admin[5] = "1234";

void reset(){
	EEPROMclear();
	for(int j = 1; j < 6; j++){ // adding the admin account!
		EEPROM.update(j, admin[j-1]);
		EEPROM.update(j+10, admin[j-1]);
	}
}

void EEPROMclear(){
	EEPROM.update(0, 0);
	for(int i=1; i<1021; i++){
		EEPROM.update(i, ' ');
	}
	EEPROM.update(1023, 1);
}

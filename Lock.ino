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
	pinMode(A1, INPUT);
}

void loop(){
	Ready();
	key = 'E';
	key = keypad.getKey();
	if(EEPROM.read(0) == 0){reset();} // Auto Reset
	if(key == 'A'){
		bool lo = login();
		if(lo){
			if(EEPROM.read(0) == 0){ChangePWD();}
			lcd1.clear();
			while(true){
				lcd1.setCursor(0, 0);
				lcd1.print("1: Open Gate");
				lcd1.setCursor(0, 1);
				lcd1.print("2: + / - Users");
				char res = 'E';
				while(res == 'E'){res = keypad.getKey();}
				if(res == 'B'){
					lcd1.clear();
					break;
				}
				if(res == '1'){gateman();}
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
							else{MemoryFull();}
							break;
						}
						if(res == '2'){
							RemoveUser();
							break;
						}
					}
					break;
				}
			}
		}
	}
	if(analogRead(A0) > 512){
		lcd1.clear();
		lcd1.print("     Reset?");
		lcd1.setCursor(0,1);
		lcd1.print("1: Yes    2: No");
		while(true){
			key = keypad.getKey();
			if(key == '1'){
				lcd1.clear();
				lcd1.print("Running factory");
				lcd1.setCursor(0,1);
				lcd1.print("RESET!");
				reset();
				delay(1000);
				lcd1.clear();
				break;
			}
			if(key == '2'){break;}
		}
		lcd1.clear();
	}
}

void Ready(){
	lcd1.print("Aritra's Lock");
	lcd1.setCursor(8,1);
	lcd1.print("Ready...");
}

void MemoryFull(){
	lcd1.clear();
	lcd1.print("Memory Full!");
	lcd1.setCursor(0, 1);
	lcd1.print("No more New User");
	delay(1000);
	lcd1.clear();	
}

void ChangePWD(){ // For ADMIN account only!
	String to = takeInput("New Password:");
	char a;
	for(int st = 11; st <= to.length()+11; st++){
		a = to[st-11];
		EEPROM.write(st, a);
	}
	lcd1.clear();
	lcd1.print("Password Changed");
	delay(1000);
	lcd1.clear();
	EEPROM.write(0, 1);
}

void AddUser(){
	int n = 1;
	bool WeDo = false;
	while(n < 1010){
		if(EEPROM.read(n) == ' '){
			WeDo = true;
			break;
		}
		n = n + 20;
	}
	if(WeDo){
		int st = n;
		while(true){
			String name = takeInput("New Username: ");
			int is = matchName(name);
			if(is > 0){
				lcd1.print("Username Occupied!");
				lcd1.setCursor(0,1);
				lcd1.print("Try another !!");
				delay(1000);
				lcd1.clear();
			}
			else{
				char a;
				for(int a1 = 0; a1 <= name.length(); a1++){
					a = name[a1];
					EEPROM.write(st, a);
					st++;
				}
				st = n+10;
				String pwd = takeInput("New Password: ");
				for(int a2 = 0; a2 <= pwd.length(); a2++){
					a = pwd[a2];
					EEPROM.write(st, a);
					st++;
				}
				lcd1.print("New User Created");
				delay(1000);
				lcd1.clear();
				break;
			}
		}
	}
	else{
		EEPROM.write(1023,0);
		MemoryFull();
	}
}

void RemoveUser(){
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
			lcd1.print("Account Deletion");
			lcd1.setCursor(0, 1);
			lcd1.print("Succesful !");
			delay(1000);
			lcd1.clear();
		}
		else{
			lcd1.clear();
			lcd1.print("Account Deletion");
			lcd1.setCursor(8, 1);
			lcd1.print("   Aborted!");
			delay(1000);
			lcd1.clear();
		}
	}
	else{
		lcd1.clear();
		lcd1.print("No Such User!");
		delay(1000);
		lcd1.clear();
	}
}

bool login(){
	String name = takeInput("Username:");
	int a = matchName(name);
	if(a > 0){
		String pwd = takeInput("Password:");
		return matchPwd(pwd, a);
	}
	else{return WUsn();}
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
	delay(1000);
	lcd1.clear();
	lcd1.print("Are you Sure?");
	lcd1.setCursor(0,1);
	lcd1.print("1: Yes    2: No");
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
				delay(1000);
				got = takeInput(ask);
			}
			return got;
		}
		if(res2 == '2'){
			return takeInput(ask);
		}
	}
}

int matchName(String name){
	String comp;
	int r = 1;
	char k = EEPROM.read(r);
	int ret = 0;
	while(r < 1021){
		comp = " ";
		while(k != ' '){
			comp.concat(k);
            r++;
            k = EEPROM.read(r);
		}
        comp.trim();
		comp.remove(comp.length()-1,1);
        name.replace(" ","");
		if(strcomp(name, comp)){
			ret = (r-(r%10)+11);
			return ret;
			break;
		}
		else{r = (r-(r%10)+21);}
	}
	if(ret == 0){
		return ret;
	}
}

bool matchPwd(String pwd, int r){
	String comp = " ";
	char k = EEPROM.read(r);
	while(k != ' '){
		comp.concat(k);
		r++;
		k = EEPROM.read(r);
	}
	comp.trim();
	comp.remove(comp.length()-1,1);
    pwd.replace(" ","");
	if(strcomp(pwd, comp)){return true;}
	return WPwd();
}

void gateman(){
	lcd1.clear();
	lcd1.print("Gate Open!");
	lcd1.setCursor(0,1);
	lcd1.print("B: Close Gate!");
	char keyL = 'E';
	while(keyL != 'B'){keyL = keypad.getKey();}
	lcd1.clear();
	lcd1.print("Closing Gate!");
	delay(1000);
	lcd1.clear();
}

bool WPwd(){
	lcd1.clear();
	lcd1.print("Invalid Password");
	lcd1.setCursor(1, 1);
	lcd1.print("Try Again Later");
	delay(1000);
	lcd1.clear();
	return false;
}

bool WUsn(){
	lcd1.clear();
	lcd1.print("Invalid Username");
	lcd1.setCursor(1, 1);
	lcd1.print("Try Again Later");
	delay(1000);
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


bool strcomp(String a, String b){
	bool ret = true;
	if(a.length() == b.length()){
		for(int i = 0; i < a.length(); i++){
			if(a[i] != b[i]){
				ret = false;
				break;
			}
		}
	}
	else{ret = false; }
	return ret;
}
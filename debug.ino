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
	Serial.begin(9600);
}


void loop(){
	Ready();
	key = 'E';
	key = keypad.getKey();
	if(EEPROM.read(0) == 0){reset();}
	if(key != 'E'){
		if(key == 'A'){
			//Serial.clear();
			//Serial.setCursor(0, 0);
			Serial.println("1: Open Gate");
			//Serial.setCursor(0, 1);
			Serial.println("2: + / - Users");
			while(true){
				char res = 'E';
				while(res == 'E'){res = keypad.getKey();}
				if(res == '1'){
					bool lo = login();
					if(lo){gateman();}
					else{
						//Serial.clear();
						Serial.println("Pehli Fursat");
						//Serial.setCursor(1,6);
						Serial.println("Me Nikal");
						delay(2000);
						//Serial.clear();
					}
					break;
				}
				if(res == '2'){
					//Serial.clear();
					//Serial.setCursor(0, 0);
					Serial.println("1: Add User");
					//Serial.setCursor(0, 1);
					Serial.println("2: Remove User");					
					while(true){
						res = 'E';
						while(res == 'E'){res = keypad.getKey();}
						if(res == '1'){
							if(EEPROM.read(1023) == 1){
                                AddUser();
                            }
							else{
								//Serial.clear();
								Serial.println("Memory Full!");
								//Serial.setCursor(0, 1);
								Serial.println("No more new User");
								delay(2000);
								//Serial.clear();
							}
                            break;
						}
						if(res == '2'){
                            RemoveUser();
                            break;
                        }
					}
				}
			}
		}
	}
}

void Ready(){
	Serial.println("Aritra's Lock");
	//Serial.setCursor(8,1);
	Serial.println("Ready...");
}

void AddUser(){
	//Serial.clear();
	Serial.println("To Add User");
	//Serial.setCursor(3, 1);
	Serial.println("Login First!");
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
				String name = takeInput("New Username: ");
				int is = matchName(name);
				if(is > 0){
					Serial.println("Username Occupied!");
					//Serial.setCursor(0,1);
					Serial.println("Try another !!");
					delay(1500);
					//Serial.clear();
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
					Serial.println("New User Created");
					delay(750);
					//Serial.clear();
					break;
				}
			}
		}
	}
	else{
		//Serial.clear();
		Serial.println("You cant do this");
		//Serial.setCursor(0,1);
		Serial.println("without login");
		delay(2000);
		//Serial.clear();
	}
}

void RemoveUser(){
	//Serial.clear();
	Serial.println("To Remove User,");
	//Serial.setCursor(3, 1);
	Serial.println("Login first!");
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
				//Serial.clear();
				Serial.println("Account Deleted");
				//Serial.setCursor(0, 1);
				Serial.println("Succesfully!");
				delay(2000);
				//Serial.clear();
			}
			else{
				//Serial.clear();
				Serial.println("Account Deletion");
				//Serial.setCursor(8, 1);
				Serial.println("Aborted!");
				delay(2000);
				//Serial.clear();
			}
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
	else{
		//Serial.clear();
		Serial.println("Wrong Username");
		delay(2000);
		//Serial.clear();
		return false;
	}
}

String takeInput(String ask){
	String got = "";
	//Serial.clear();
	Serial.println(ask);
	//Serial.setCursor(0, 1);
	//Serial.cursor();
	//Serial.blink();
	char res2;
	while(true){
		res2 = keypad.getKey();
		if(res2){
			if(res2 == 'A'){break;}
			//if(res2 == 'B'){;}            //tried to have a backspace option
			Serial.println(res2);
			got.concat(res2);
		}
	}
	//Serial.noBlink();
	//Serial.noCursor();
	//Serial.clear();
	Serial.println("You Entered: ");
	//Serial.setCursor(0, 1);
	Serial.println(got);
	delay(1500);
	//Serial.clear();
	Serial.println("Are you Sure?");
	//Serial.setCursor(0,1);
	Serial.println("1: Yes    2: No");
	delay(2000);
	while(true){
		res2 = keypad.getKey();
		if(res2 == '1'){
			if(got.length()>9){
				Serial.println("Maximum length");
				//Serial.setCursor(1,0);
				Serial.println("should be 9");
				delay(2000);
				//Serial.clear();
				Serial.println("  Try Again !!  ");
				delay(500);
				got = takeInput(ask);
			}
			//Serial.clear();
			Serial.println("Returning Text");
			delay(2000);
			//Serial.clear();
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
	char k ='E';
	int ret = 0;
	while(r < 1021){
		comp = " ";
		while(k != ' '){
			k = EEPROM.read(r);
			comp.concat(k);
			r++;
		}
		comp.trim();
		if(name.equals(comp)){
			ret = (r-(r%10)+11);
			//Serial.clear();
			Serial.println("Username Match");
			delay(2000);
			//Serial.clear();
			return ret;
			break;
		}
		else{r = (r-(r%10)+21);}
	}
	if(ret == 0){
		//Serial.clear();
		Serial.println("No match found");
		delay(2000);
		//Serial.clear();
		return ret;
	}
}

bool matchPwd(String pwd, int r){
	String comp = " ";
	char k = 'E';
	bool ret = false;
	while(k != ' '){
		k = EEPROM.read(r);
		comp.concat(k);
		r++;
	}
	comp.trim();
	if(pwd.equals(comp)){ret = true;}
	else{return ret;}
}

void gateman(){
	//Serial.clear();
	Serial.println("Gate Open!");
	//Serial.setCursor(0,1);
	Serial.println("B: Close Gate!");
	char keyL = 'E';
	while(keyL != 'B'){keypad.getKey();}
	//Serial.clear();
	Serial.println("Closing Gate!");
	delay(500);
	//Serial.clear();
}

bool WPwd(){
	//Serial.clear();
	Serial.println("Invalid Password");
	//Serial.setCursor(1, 1);
	Serial.println("Try Again Later");
	delay(2000);
	//Serial.clear();
	return false;
}

bool WUsn(){
	//Serial.clear();
	Serial.println("Invalid Username");
	//Serial.setCursor(1, 1);
	Serial.println("Try Again Later");
	delay(2000);
	//Serial.clear();
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

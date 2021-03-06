#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <Servo.h>

/*
User Instructions:-
1) Pressing A helps to enter into input mode for
	USERNAME and also acts as the "ENTER" key!
2) Presssing B helps it to lock if it is unlocked!
3) Username and password should be:
	minimum 4 character and
	maximum 9 characters long
4) Passwords and Usernames are all numeric only
5) Maximum number of users supported = 50 (+1 admin)
6) Admin Username and Password both are "1234" initially.
	Password have to be changed on first Login.
7) Controls for giving input:
	numbers	= > 	to type input
	'*' 	= > 	to move cursor to the left
	'#' 	= > 	to move cursor to the right
	'C' 	= > 	to have a backspace
8) This device can be setup for two types of users:
	a) Home user:- every user is an admin user and can add or remove users
	b) Office Users:- Only the admin user can add or remove users

9) Hidden Feature for Sincere People who care to read this user guide:
	(points 5 and 6 will be printed on the box for advertisement purpose)
	a) To change your password:
		When device is READY....
		Press these keys successively => *, #, C and B
	b) There was one more hidden feature, but I removed
		it in this commit, go to previous commits to see it!

Note:- Feel free to raise any issues if you feel like
	at "https://github.com/PeithonKing/Lock/issues".
	It will be addressed ASAP.

					*** Thanks ***
*/

//            *** DON'T FORGET (Not for users, for developers) ***
// EEPROM space [0] stores if admin password has been
// changed or not. If no, value is 0, else value is 1.
// ////////// If you see 0 at [0] remember to reset

// EEPROM space [1023] stores current number of users (excluding admin)
// EEPROM space [1022] stores current mode of use:
//        mode = 1     =>     for home use
//        mode = 2     =>     for office use
// unused EEPROM space = [1021] which should be kept = ' '
// All the rest of the spaces are used in the following ways:
//		starting from [1], consecutive 20 spaces are available for users
//		the first 9 spaces for username, then a ' ' (space)
//		next 9 spaces for password, then a ' ' again
//		thus all the multiple of 10 places (Eg: [10], [20], [150], [1110])
// 		have values ' ' for propper functioning
//		admin credentials are stored in [1:21] spaces
//		all the other spaces are ' ' then
//		first new user credentials are stored from [21] onwards, upto [40]
//	 	next user comes in at [41] and the process goes on
// 		when an user is removed, its place gets overwritten by ' 's

// All the spaces except [0], [1022], [1023] are char type
// So, while reading them never try to read them in int,
// to store them, only use char type variables
// [0], [1022], and [1023] have to be read in int type vaiable!


LiquidCrystal lcd1(0, 1, 10, 11, 12, 13);
int ServoPin = A1;
int open = 92;
int close = 0;

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

Servo gate;

void setup(){
	lcd1.begin(16,2);
	pinMode(A0, INPUT);
	gate.attach(ServoPin);
		gate.write(close);
}

int pwdc = 0;
bool canEdit = false;

void loop(){
	Ready();
	if(EEPROM.read(0) == 0){reset();} // Auto Reset
	key = keypad.getKey();
	if(key == 'A'){
		pwdc = 0;
		bool lo = login();
		if(lo){
			if(EEPROM.read(0) == 0){
				ChangePWDadmin();
				lcd1.clear();
				lcd1.print("  Environment:");
				lcd1.setCursor(0,1);
				lcd1.print("1:Home  2:Office");
				while(true){
					key = keypad.getKey();
					if(key == '1'){
						EEPROM.update(1022, 1);
						break;
					}
					if(key == '2'){
						EEPROM.update(1022, 2);
						break;
					}
				}
				lcd1.clear();
			}
			if(EEPROM.read(1022) == 1){adminUser();} // home user
			if(EEPROM.read(1022) == 2){ // office user
				if(canEdit){
					adminUser();
				}
				else{normalUser();}
			}
		}
	}
	if(analogRead(A0) > 500){callReset();}
	if(key == '*' && pwdc == 0){pwdc++;}
	if(key == '#' && pwdc == 1){pwdc++;}
	if(key == 'C' && pwdc == 2){pwdc++;}
	if(key == 'B' && pwdc == 3){
		ChangePWD();
		pwdc = 0;
	}
	if(key != '*' && key != '#' && key != 'B' && key != 'C' && key != 0){pwdc = 0;}
	if(key == '0'){
		lcd1.clear();
		lcd1.print("Number of Users:");
		lcd1.setCursor(0,1);
		lcd1.print("1 Admin + ");
		lcd1.print(EEPROM.read(1023));
		lcd1.print(" oth");
		delay(2000);
		lcd1.clear();
	}
}

void adminUser(){
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
			canEdit = false;
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
					if(EEPROM.read(1023) < 50){AddUser();}
					else{
						lcd1.clear();
						lcd1.print("Memory Full!");
						lcd1.setCursor(0, 1);
						lcd1.print("No more New User");
						delay(2000);
						lcd1.clear();
					}
					break;
				}
				if(res == '2'){
					if(EEPROM.read(1023) > 0){RemoveUser();}
					else{
						lcd1.clear();
						lcd1.print("Cannot Remove");
						lcd1.setCursor(0, 1);
						lcd1.print("the last User");
						delay(2000);
						lcd1.clear();
					}
					break;
				}
				if(res == 'B'){
					break;
				}
			}
		}
	}
}

void normalUser(){
	lcd1.clear();
	while(true){
		lcd1.setCursor(0, 0);
		lcd1.print("1: Open Gate");
		lcd1.setCursor(0, 1);
		lcd1.print("B: Logout");
		char res = 'E';
		while(res == 'E'){res = keypad.getKey();}
		if(res == 'B'){
			lcd1.clear();
			break;
		}
		if(res == '1'){gateman();}
	}
}

void Ready(){
	lcd1.print("Aritra's Lock");
	lcd1.setCursor(8,1);
	lcd1.print("Ready...");
}

void ChangePWDadmin(){ // For ADMIN account only!
	for(int st = 11; st < 20; st++){EEPROM.update(st, ' ');}
	String pw1;
	while(true){
		pw1 = takeInput("New Password:");
		String pw2 = takeInput("Re Enter:");
		if(strcomp(pw1, pw2)){break;}
		else{
			lcd1.clear();
			lcd1.print("Didn't match!");
			lcd1.setCursor(0,1);
			lcd1.print("  Try Again!!");
			delay(1000);
			lcd1.clear();
		}
	}
	char a;
	for(int st = 11; st <= pw1.length()+11; st++){
		a = pw1[st-11];
		EEPROM.write(st, a);
	}
	lcd1.clear();
	lcd1.print("Password Changed");
	delay(1000);
	lcd1.clear();
	EEPROM.write(0, 1);
}

void ChangePWD(){ //For all accounts incluing ADMIN account.
	String name = takeInput("Username:");
	int a = matchName(name);
	if(a > 0){
		String pwd = takeInput("Old Password:");
		if(matchPwd(pwd, a)){
			String pw1;
			while(true){
				pw1 = takeInput("New Password:");
				String pw2 = takeInput("Re Enter:");
				if(strcomp(pw1, pw2)){break;}
				else{
					lcd1.clear();
					lcd1.print("Didn't match!");
					lcd1.setCursor(0,1);
					lcd1.print("  Try Again!!");
					delay(1000);
					lcd1.clear();
				}
			}
			char b;
			for(int st = a; st < (a+10); st++){EEPROM.update(st, ' ');}
			for(int st = a; st <= (pw1.length()+a); st++){
				b = pw1[st-a];
				EEPROM.write(st, b);
			}
			lcd1.clear();
			lcd1.print("Password Changed");
			delay(1000);
			lcd1.clear();
		}
		else{WPwd();}
	}
	else{WUsn();}
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
				lcd1.clear();
				lcd1.print("Name Occupied!");
				lcd1.setCursor(0,1);
				lcd1.print("Try another !!");
				delay(2000);
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
				lcd1.clear();
				lcd1.print("New User Created");
				int numb = EEPROM.read(1023);
				EEPROM.write(1023,(numb+1));
				delay(1000);
				lcd1.clear();
				break;
			}
		}
	}
	else if(EEPROM.read(1022) != 50){
		lcd1.clear();
		lcd1.print("Device ");
		lcd1.setCursor(0, 1);
		lcd1.print("   Malfunction");
		delay(1500);
		callReset();
	}
}

void RemoveUser(){
	String name = takeInput("Name to remove:");
	int a = matchName(name);
	if(a == 11){
		lcd1.clear();
		lcd1.print("Cannot delete,");
		lcd1.setCursor(0, 1);
		lcd1.print("  Admin account");
		delay(1500);
		lcd1.clear();
	}
	else if(a > 0){
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
			lcd1.print("  Succesful !");
			int numb = EEPROM.read(1023);
			EEPROM.write(1023,(numb-1));
			delay(1500);
			lcd1.clear();
		}
		else{
			lcd1.clear();
			lcd1.print("Account Deletion");
			lcd1.setCursor(8, 1);
			lcd1.print("Aborted!");
			delay(1500);
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

/*
String takeInput(String ask){ // verify wala takeInput() function
	String got = "";
	int cursPos = 0;
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
			if(res2 == 'C'){
				got.remove(cursPos-1,1);
				cursPos--;
				lcd1.clear();
				lcd1.print(ask);
				lcd1.setCursor(0, 1);
				lcd1.print(got);
				lcd1.setCursor(cursPos, 1);
			}
			else if(res2 == '*'){   // move cursor to left  (if possible)
				if(cursPos > 0){
					cursPos--;
					lcd1.setCursor(cursPos, 1);
				}
			}
			else if(res2 == '#'){   // move cursor to right (if possible)
				if(cursPos < got.length()){
					cursPos++;
					lcd1.setCursor(cursPos, 1);
				}
			}
			else if(res2 == 'B' || res2 == 'D'){}
			else{
				if(cursPos == got.length()){
					lcd1.print(res2);
					got.concat(res2);
					cursPos++;
				}
				else{
					got = insert(got, cursPos, res2);
					cursPos++;
					lcd1.clear();
					lcd1.print(ask);
					lcd1.setCursor(0, 1);
					lcd1.print(got);
					lcd1.setCursor(cursPos, 1);
				}
			}
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
			if(got.length()>9 || got.length() < 4){
				lcd1.clear();
				lcd1.print("Incompatible");
				lcd1.setCursor(9,1);
				lcd1.print("length");
				delay(1500);
				lcd1.clear();
				lcd1.print("Max length = 9");
				lcd1.setCursor(0,1);
				lcd1.print("Min length = 4");
				delay(2000);
				lcd1.clear();
				lcd1.print("Read User Guide");
				lcd1.setCursor(0,1);
				lcd1.print("and Try Again!");
				delay(2000);
				got = takeInput(ask);
			}
			return got;
		}
		if(res2 == '2'){
			return takeInput(ask);
		}
	}
}
*/

String takeInput(String ask){  // direct wala takeInput() function
	String got = "";
	int cursPos = 0;
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
			if(res2 == 'C'){
				got.remove(cursPos-1,1);
				cursPos--;
				lcd1.clear();
				lcd1.print(ask);
				lcd1.setCursor(0, 1);
				lcd1.print(got);
				lcd1.setCursor(cursPos, 1);
			}
			else if(res2 == '*'){   // move cursor to left  (if possible)
				if(cursPos > 0){
					cursPos--;
					lcd1.setCursor(cursPos, 1);
				}
			}
			else if(res2 == '#'){   // move cursor to right (if possible)
				if(cursPos < got.length()){
					cursPos++;
					lcd1.setCursor(cursPos, 1);
				}
			}
			else if(res2 == 'B' || res2 == 'D'){}
			else{
				if(cursPos == got.length()){
					lcd1.print(res2);
					got.concat(res2);
					cursPos++;
				}
				else{
					got = insert(got, cursPos, res2);
					cursPos++;
					lcd1.clear();
					lcd1.print(ask);
					lcd1.setCursor(0, 1);
					lcd1.print(got);
					lcd1.setCursor(cursPos, 1);
				}
			}
		}
	}
	lcd1.noBlink();
	lcd1.noCursor();
	// lcd1.clear();
	// lcd1.print("You Entered: ");
	// lcd1.setCursor(0, 1);
	// lcd1.print(got);
	// delay(500);
	// lcd1.clear();
	if(got.length()>9 || got.length() < 4){
		lcd1.clear();
		lcd1.print("Incompatible");
		lcd1.setCursor(9,1);
		lcd1.print("length");
		delay(500);
		lcd1.clear();
		lcd1.print("Max length = 9");
		lcd1.setCursor(0,1);
		lcd1.print("Min length = 4");
		delay(1000);
		lcd1.clear();
		lcd1.print("Read User Guide");
		lcd1.setCursor(0,1);
		lcd1.print("and Try Again!");
		delay(500);
		got = takeInput(ask);
	}
	return got;
}

String insert(String got, int p, char c){
	int g = got.length();
	String part1 = "";
	int i = 0;
	for(i; i<p; i++){
		part1.concat(got[i]);
	}
	part1.concat(c);
	for(i; i<g; i++){
		part1.concat(got[i]);
	}
	return part1;
}

int matchName(String name){
	String comp;
	int r = 0;
	int s = 1;
	char k;
	int ret = 0;
	while(s < 1021){
		comp = " ";
		k = EEPROM.read(s);
		while(k != ' '){
			comp.concat(k);
						s++;
						k = EEPROM.read(s);
		}
				comp.trim();
		comp.remove(comp.length()-1,1);
		if(strcomp(name, comp)){
			ret = ((20*r)+11);
			return ret;
			break;
		}
		r++;
		s = ((20*r)+1);
	}
	if(ret == 0){
		return ret;
	}
}

bool matchPwd(String pwd, int back){
	int r = back;
	String comp = " ";
	char k = EEPROM.read(r);
	while(k != ' '){
		comp.concat(k);
		r++;
		k = EEPROM.read(r);
	}
	comp.trim();
	comp.remove(comp.length()-1,1);
	if(strcomp(pwd, comp)){
		if(back == 11){canEdit = true;}
		return true;
	}
	return WPwd();
}

void gateman(){
	lcd1.clear();
	lcd1.print("Opening Gate");
	gate.write(open);
	delay(100);
	lcd1.clear();
	lcd1.print("Gate Open!");
	lcd1.setCursor(0,1);
	lcd1.print("B: Close Gate!");
	char keyL = 'E';
	while(keyL != 'B'){keyL = keypad.getKey();}
	lcd1.clear();
	lcd1.print("Closing Gate!");
	gate.write(close);
	delay(100);
	lcd1.clear();
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

bool WUsn(){
	lcd1.clear();
	lcd1.print("Invalid Username");
	lcd1.setCursor(1, 1);
	lcd1.print("Try Again Later");
	delay(2000);
	lcd1.clear();
	return false;
}

char admin[5] = "1234";

void callReset(){
	lcd1.clear();
	lcd1.print("     Reset?");
	lcd1.setCursor(0,1);
	lcd1.print("1: Yes    2: No");
	char key;
	while(true){
		key = keypad.getKey();
		if(key == '1'){
			lcd1.clear();
			lcd1.print("This will remove");
			lcd1.setCursor(0,1);
			lcd1.print("   all your data");
			delay(2000);
			lcd1.clear();
			lcd1.print("    Proceed?");
			lcd1.setCursor(0,1);
			lcd1.print("1: Yes    2: No");
			char key2;
			while(true){
				key2 = keypad.getKey();
				if(key2 == '1'){
					lcd1.clear();
					lcd1.print("Running factory");
					lcd1.setCursor(0,1);
					lcd1.print("RESET!");
					reset();
					delay(2000);
					lcd1.clear();
					break;
				}
				if(key2 == '2'){break;}
			}
			break;
		}
		if(key == '2'){break;}
	}
	lcd1.clear();
}

void reset(){
	EEPROMclear();
	for(int j = 1; j < 6; j++){ // adding the admin account!
		EEPROM.update(j, admin[j-1]);
		EEPROM.update(j+10, admin[j-1]);
	}
	gate.write(close);
	pwdc = 0;
}

void EEPROMclear(){
	EEPROM.update(0, 0);
	for(int i=1; i<1022; i++){
		EEPROM.update(i, ' ');
	}
	EEPROM.update(1023, 0);
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
// Эта инструкция обязательно должна быть первой, т.к. этот код компилируется в бинарный,
// и загрузчик передает управление по адресу первой инструкции бинарного образа ядра ОС.
// Начало файла kernel.cpp
extern "C" int kmain();
__declspec(naked) void startup() {
	__asm 
	{
		call kmain;
	}
} ;

#define VIDEO_BUF_PTR (0xb8000)
#define IDT_TYPE_INTR (0x0E)
#define IDT_TYPE_TRAP (0x0F)
#define CURSOR_PORT (0x3D4)
#define VIDEO_WIDTH (80)
#define PIC1_PORT (0x20)
// Селектор секции кода, установленный загрузчиком ОС
#define GDT_CS (0x8)
char str_in[81] = { 0 };
char command[41] = { 0 };
int str_in_count = 0;
char color = 0;
int cur_strnum = 0, cur_position = 0;
const char* sys_symbols = "0123456789abcdefghijklmnopqrstuvwxyz";
const char* codes_table = "??1234567890????qwertyuiop????asdfghjkl?????zxcvbnm";
bool lshift = false, rshift = false, caps = false;
void load_color();
void out_str(const char* ptr);
void cursor_moveto(unsigned int strnum, unsigned int pos);
void keyb_process_keys();
void keyb_init();
void keyb_handler();
inline void outb(unsigned short port, unsigned char data);
inline unsigned char inb(unsigned short port);
typedef void (*intr_handler)();
void intr_enable();
void intr_start();
void intr_init();
void intr_reg_handler(int num, unsigned short segm_sel, unsigned short flags, intr_handler hndlr);
void info();
void load_color();
void argv_parser(const char* str);
void on_key(unsigned char scan_code);
void fill_the_string(unsigned char scan_code);
void out_char(unsigned char symbol);
bool strcmp(char* string1, const char* string2);
unsigned int strlen(char* string);
void strrev(char* S);
unsigned int atoi(char* num);
long long int atoi64(char* string);
unsigned int pow(unsigned int num, unsigned int deg);
void welcome_string();
void com_symbol();
void nsconv(char* arg1, int arg2, int arg3);
unsigned int to_10ss(char* nx, unsigned int ss);
void date(long long int number, int year);
void date_to_string(long long int day, unsigned int month, unsigned int year,
	unsigned int hour, unsigned int min, unsigned int sec);
void shutdown();
void clear();
void error(int error_num);
bool correct_time_win(char* number);
bool correct_time_pos(char* number);
bool correct_number(char* number);
bool correct_nsconv(char* number, unsigned int from_system, unsigned int to_system);
bool overflow_ll(char* number);
bool overflow_uint(char* number);
bool correct_ns(char* number, int ns);

#ifdef _M_IX86 // use this file only for 32-bit architecture

#define CRT_LOWORD(x) dword ptr [x+0]
#define CRT_HIWORD(x) dword ptr [x+4]

extern "C"
{
	__declspec(naked) void _alldiv()
	{
#define DVND    esp + 16      // stack address of dividend (a)
#define DVSR    esp + 24      // stack address of divisor (b)

		__asm
		{
			push    edi
			push    esi
			push    ebx

			xor edi, edi; result sign assumed positive

			mov     eax, CRT_HIWORD(DVND); hi word of a
			or eax, eax; test to see if signed
			jge     short L1; skip rest if a is already positive
			inc     edi; complement result sign flag
			mov     edx, CRT_LOWORD(DVND); lo word of a
			neg     eax; make a positive
			neg     edx
			sbb     eax, 0
			mov     CRT_HIWORD(DVND), eax; save positive value
			mov     CRT_LOWORD(DVND), edx
			L1 :
			mov     eax, CRT_HIWORD(DVSR); hi word of b
				or eax, eax; test to see if signed
				jge     short L2; skip rest if b is already positive
				inc     edi; complement the result sign flag
				mov     edx, CRT_LOWORD(DVSR); lo word of a
				neg     eax; make b positive
				neg     edx
				sbb     eax, 0
				mov     CRT_HIWORD(DVSR), eax; save positive value
				mov     CRT_LOWORD(DVSR), edx
				L2 :


			or eax, eax; check to see if divisor < 4194304K
				jnz     short L3; nope, gotta do this the hard way
				mov     ecx, CRT_LOWORD(DVSR); load divisor
				mov     eax, CRT_HIWORD(DVND); load high word of dividend
				xor edx, edx
				div     ecx; eax < -high order bits of quotient
				mov     ebx, eax; save high bits of quotient
				mov     eax, CRT_LOWORD(DVND); edx:eax < -remainder : lo word of dividend
				div     ecx; eax < -low order bits of quotient
				mov     edx, ebx; edx:eax < -quotient
				jmp     short L4; set sign, restore stack and return


				L3:
			mov     ebx, eax; ebx:ecx < -divisor
				mov     ecx, CRT_LOWORD(DVSR)
				mov     edx, CRT_HIWORD(DVND); edx:eax < -dividend
				mov     eax, CRT_LOWORD(DVND)
				L5 :
				shr     ebx, 1; shift divisor right one bit
				rcr     ecx, 1
				shr     edx, 1; shift dividend right one bit
				rcr     eax, 1
				or ebx, ebx
				jnz     short L5; loop until divisor < 4194304K
				div     ecx; now divide, ignore remainder
				mov     esi, eax; save quotient


				mul     CRT_HIWORD(DVSR); QUOT* CRT_HIWORD(DVSR)
				mov     ecx, eax
				mov     eax, CRT_LOWORD(DVSR)
				mul     esi; QUOT* CRT_LOWORD(DVSR)
				add     edx, ecx; EDX:EAX = QUOT * DVSR
				jc      short L6; carry means Quotient is off by 1


				cmp     edx, CRT_HIWORD(DVND); compare hi words of result and original
				ja      short L6; if result > original, do subtract
				jb      short L7; if result < original, we are ok
				cmp     eax, CRT_LOWORD(DVND); hi words are equal, compare lo words
				jbe     short L7; if less or equal we are ok, else subtract
				L6 :
			dec     esi; subtract 1 from quotient
				L7 :
			xor edx, edx; edx:eax < -quotient
				mov     eax, esi


				L4 :
			dec     edi; check to see if result is negative
				jnz     short L8; if EDI == 0, result should be negative
				neg     edx; otherwise, negate the result
				neg     eax
				sbb     edx, 0


				L8:
			pop     ebx
				pop     esi
				pop     edi

				ret     16
		}

#undef DVND
#undef DVSR
	}

	__declspec(naked) void _allmul()
	{
#define A       esp + 8       // stack address of a
#define B       esp + 16      // stack address of b

		__asm
		{
			push    ebx

			mov     eax, CRT_HIWORD(A)
			mov     ecx, CRT_LOWORD(B)
			mul     ecx; eax has AHI, ecx has BLO, so AHI* BLO
			mov     ebx, eax; save result

			mov     eax, CRT_LOWORD(A)
			mul     CRT_HIWORD(B); ALO* BHI
			add     ebx, eax; ebx = ((ALO * BHI) + (AHI * BLO))

			mov     eax, CRT_LOWORD(A); ecx = BLO
			mul     ecx; so edx : eax = ALO * BLO
			add     edx, ebx; now edx has all the LO* HI stuff

			pop     ebx

			ret     16; callee restores the stack
		}

#undef A
#undef B
	}

	__declspec(naked) void _allrem()
	{
#define DVND    esp + 12      // stack address of dividend (a)
#define DVSR    esp + 20      // stack address of divisor (b)

		__asm
		{
			push    ebx
			push    edi


			; Determine sign of the result(edi = 0 if result is positive, non - zero
			; otherwise) and make operands positive.

			xor edi, edi; result sign assumed positive

			mov     eax, CRT_HIWORD(DVND); hi word of a
			or eax, eax; test to see if signed
			jge     short L1; skip rest if a is already positive
			inc     edi; complement result sign flag bit
			mov     edx, CRT_LOWORD(DVND); lo word of a
			neg     eax; make a positive
			neg     edx
			sbb     eax, 0
			mov     CRT_HIWORD(DVND), eax; save positive value
			mov     CRT_LOWORD(DVND), edx
			L1 :
			mov     eax, CRT_HIWORD(DVSR); hi word of b
				or eax, eax; test to see if signed
				jge     short L2; skip rest if b is already positive
				mov     edx, CRT_LOWORD(DVSR); lo word of b
				neg     eax; make b positive
				neg     edx
				sbb     eax, 0
				mov     CRT_HIWORD(DVSR), eax; save positive value
				mov     CRT_LOWORD(DVSR), edx
				L2 :

			;
			; Now do the divide.First look to see if the divisor is less than 4194304K.
				; If so, then we can use a simple algorithm with word divides, otherwise
				; things get a little more complex.
				;
			; NOTE - eax currently contains the high order word of DVSR
				;

			or eax, eax; check to see if divisor < 4194304K
				jnz     short L3; nope, gotta do this the hard way
				mov     ecx, CRT_LOWORD(DVSR); load divisor
				mov     eax, CRT_HIWORD(DVND); load high word of dividend
				xor edx, edx
				div     ecx; edx < -remainder
				mov     eax, CRT_LOWORD(DVND); edx:eax < -remainder : lo word of dividend
				div     ecx; edx < -final remainder
				mov     eax, edx; edx:eax < -remainder
				xor edx, edx
				dec     edi; check result sign flag
				jns     short L4; negate result, restore stack and return
				jmp     short L8; result sign ok, restore stack and return

				;
			; Here we do it the hard way.Remember, eax contains the high word of DVSR
				;

		L3:
			mov     ebx, eax; ebx:ecx < -divisor
				mov     ecx, CRT_LOWORD(DVSR)
				mov     edx, CRT_HIWORD(DVND); edx:eax < -dividend
				mov     eax, CRT_LOWORD(DVND)
				L5 :
				shr     ebx, 1; shift divisor right one bit
				rcr     ecx, 1
				shr     edx, 1; shift dividend right one bit
				rcr     eax, 1
				or ebx, ebx
				jnz     short L5; loop until divisor < 4194304K
				div     ecx; now divide, ignore remainder

				;
			; We may be off by one, so to check, we will multiply the quotient
				; by the divisor and check the result against the orignal dividend
				; Note that we must also check for overflow, which can occur if the
				; dividend is close to 2 * *64 and the quotient is off by 1.
				;

			mov     ecx, eax; save a copy of quotient in ECX
				mul     CRT_HIWORD(DVSR)
				xchg    ecx, eax; save product, get quotient in EAX
				mul     CRT_LOWORD(DVSR)
				add     edx, ecx; EDX:EAX = QUOT * DVSR
				jc      short L6; carry means Quotient is off by 1

				;
			; do long compare here between original dividend and the result of the
				; multiply in edx : eax.If original is larger or equal, we are ok, otherwise
				; subtract the original divisor from the result.
				;

			cmp     edx, CRT_HIWORD(DVND); compare hi words of result and original
				ja      short L6; if result > original, do subtract
				jb      short L7; if result < original, we are ok
				cmp     eax, CRT_LOWORD(DVND); hi words are equal, compare lo words
				jbe     short L7; if less or equal we are ok, else subtract
				L6 :
			sub     eax, CRT_LOWORD(DVSR); subtract divisor from result
				sbb     edx, CRT_HIWORD(DVSR)
				L7:

			;
			; Calculate remainder by subtracting the result from the original dividend.
				; Since the result is already in a register, we will do the subtract in the
				; opposite direction and negate the result if necessary.
				;

			sub     eax, CRT_LOWORD(DVND); subtract dividend from result
				sbb     edx, CRT_HIWORD(DVND)

				;
			; Now check the result sign flag to see if the result is supposed to be positive
				; or negative.It is currently negated(because we subtracted in the 'wrong'
					; direction), so if the sign flag is set we are done, otherwise we must negate
				; the result to make it positive again.
				;

			dec     edi; check result sign flag
				jns     short L8; result is ok, restore stack and return
				L4:
			neg     edx; otherwise, negate the result
				neg     eax
				sbb     edx, 0

				;
			; Just the cleanup left to do.edx:eax contains the quotient.
				; Restore the saved registers and return.
				;

		L8:
			pop     edi
				pop     ebx

				ret     16
		}

#undef DVND
#undef DVSR
	}

#undef CRT_LOWORD
#undef CRT_HIWORD
#endif
}






// Структура описывает данные об обработчике прерывания
#pragma pack(push, 1) // Выравнивание членов структуры запрещено 
struct idt_entry {
	unsigned short base_lo; // Младшие биты адреса обработчика
	unsigned short segm_sel; // Селектор сегмента кода
	unsigned char always0; // Этот байт всегда 0
	unsigned char flags; // Флаги тип. Флаги: P, DPL, Типы - это константы - IDT_TYPE...
	unsigned short base_hi; // Старшие биты адреса обработчика
};
// Структура, адрес которой передается как аргумент команды lidt
struct idt_ptr {
	unsigned short limit;
	unsigned int base;
};
#pragma pack(pop)
__declspec(naked) void default_intr_handler() {
	__asm 
	{
		pusha
	}
	// ... (реализация обработки)
	__asm 
	{
		popa
		iretd
	}
}

struct idt_entry g_idt[256]; // Реальная таблица IDT
struct idt_ptr g_idtp; // Описатель таблицы для команды lidt


typedef void (*intr_handler)();
void intr_reg_handler(int num, unsigned short segm_sel, unsigned short flags, intr_handler hndlr) {
	unsigned int hndlr_addr = (unsigned int)hndlr;
	g_idt[num].base_lo = (unsigned short)(hndlr_addr & 0xFFFF);
	g_idt[num].segm_sel = segm_sel;
	g_idt[num].always0 = 0;
	g_idt[num].flags = flags;
	g_idt[num].base_hi = (unsigned short)(hndlr_addr >> 16);
}
// Функция инициализации системы прерываний: заполнение массива с адресами обработчиков
void intr_init() {
	int i;
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	for (i = 0; i < idt_count; i++)
		intr_reg_handler(i, GDT_CS, 0x80 | IDT_TYPE_INTR,
			default_intr_handler); // segm_sel=0x8, P=1, DPL=0, Type=Intr
}

void intr_start() {
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	g_idtp.base = (unsigned int)(&g_idt[0]);
	g_idtp.limit = (sizeof(struct idt_entry) * idt_count) - 1;
	__asm 
	{
		lidt g_idtp
	}
	//__lidt(&g_idtp);
}
void intr_enable()
{
	__asm sti;
}

__inline unsigned char inb(unsigned short port) {
	unsigned char data;
	__asm 
	{
		push dx
		mov dx, port
		in al, dx
		mov data, al
		pop dx
	}
	return data;
}

__inline void outb(unsigned short port, unsigned char data) {
	__asm 
	{
		push dx
		mov dx, port
		mov al, data
		out dx, al
		pop dx
	}
}


__declspec(naked) void keyb_handler() {
	__asm pusha;
	// Обработка поступивших данных
	keyb_process_keys();
	// Отправка контроллеру 8259 нотификации о том, что прерывание 
	// обработано.Если не отправлять нотификацию, то контроллер не будет посылать
	// новых сигналов о прерываниях до тех пор, пока ему не сообщать что
	// прерывание обработано.
	outb(PIC1_PORT, 0x20);
	__asm
	{
		popa
		iretd
	}
}


void keyb_init() {
	// Регистрация обработчика прерывания
	intr_reg_handler(0x09, GDT_CS, 0x80 | IDT_TYPE_INTR, keyb_handler);
	// segm_sel=0x8, P=1, DPL=0, Type=Intr
	// Разрешение только прерываний клавиатуры от контроллера 8259
	outb(PIC1_PORT + 1, 0xFF ^ 0x02); // 0xFF - все прерывания, 0x02 - бит IRQ1(клавиатура).
		// Разрешены будут только прерывания, чьи биты установлены в 0
}


bool correct_ns(char* number, int ns) {
	for (int i = 0; i < 34 && number[i] != '\0'; i++) {
		for (int j = 0; j < 37; j++) {
			if (number[i] == sys_symbols[j] && j >= ns)
					return false;
		}
	}
	return true;
}
bool overflow_uint(char* number) {
	const char *standart_uint = "4294967295";
	for (int i = 0; i < 10; i++) {
		if (number[i] == '\0')
			return true;
	}
	if (number[10] != '\0')
		return false;
	else {
		for (int i = 0; i < 10; i++) {
			if (number[i] > standart_uint[i])
				return false;
			//if (standart_uint[i] == '\0' && number[i] == '\0') 
				//return true;
		}
		return true;
	}
	return true;
}

bool overflow_ll(char* number) {
	const char *standart_ll = "9223372036854775807";
	for (int i = 0; i < 19; i++) {
		if (number[i] == '\0')
			return true;
	}
	if (number[19] != '\0')
		return false;
	else {
		for (int i = 0; i < 19; i++) {
			if (number[i] > standart_ll[i])
				return false;
			//if (standart_ll[i] == '\0' && number[i] == '\0')
				//return true;
		}
		return true;

	}
	return true;
}

bool correct_nsconv(char* number, unsigned int from_system, unsigned int to_system) {
	if (number[0] == '\0') {
		error(6);
		return false;
	}
	if (from_system < 2 || from_system>36 || to_system < 2 || to_system>36) {
		error(4);
		return false;
	}
	if (!overflow_uint(number)) {
		error(2);
		return false;
	}
	if (!correct_ns(number, from_system)) {
		error(3);
		return false;
	}
	return true;
}
bool correct_number(char* number) {
	for (int i = 0; i < 40; i++) {
		if (number[i] == '\0')
			return true;
		if (number[i] < '0' || number[i]>'9')
			return false;
	}
	return true;
}
bool correct_time_pos(char* number) {
	unsigned int num = 0;
	if (number[0] == '\0') {
		error(6);
		return false;
	}
	if (!correct_number(number)) {
		error(6);
		return false;
	}
	num = atoi(number);
	if (num == -1) {
		error(5);
		return false;
	}
	if (!overflow_uint(number)) {
		error(2);
		return false;
	}
	return true;
}

bool correct_time_win(char* number) {
	long long int num = 1;
	if (number[0] == '\0') {
		error(6);
		return false;
	}
	if (!correct_number(number)) {
		error(6);
		return false;
	}
	num = atoi64(number);
	if (num == -1) {
		error(5);
		return false;
	}
	if (!overflow_ll(number)) {
		error(2);
		return false;
	}
	return true;
}



unsigned int atoi(char* string) {
	int i = 0;
	unsigned int num = 0;
	for (i = 0; string[i] >= '0' && string[i] <= '9'; i++)
		num = 10 * num + (string[i] - '0');
	if (string[i] != '\0' && string[i] != ' ')
		return -1;
	return num;
}

long long int atoi64(char* str) {
	int i = 0;
	long long int num = 1;
	num--;
	for (i = 0; str[i] >= '0' && str[i] <= '9'; i++)
		num = 10 * num + (str[i] - '0');
	if (str[i] != '\0' && str[i] != ' ')
		return -1;
	return num;
}

unsigned int strlen(char* string) {
	unsigned int len = 0;
	for (int i = 0; i < 40 && string[i] != '\0'; i++)
		len++;
	return len;
}


bool strcmp(char* string1, const char* string2) {
	if (strlen(string1) != strlen((char*)string2)) return false;
	for (int i = 0; i < strlen(string1); i++) {
		if (string1[i] != string2[i]) return false;
	}
	return true;
}

void strrev(char* S) {
	int i, j, l;
	char t;
	l = strlen(S);
	i = 0;
	j = l - 1;
	while (i < j) {
		t = S[i];
		S[i] = S[j];
		S[j] = t;
		i++; j--;
	}
}

unsigned int pow(unsigned int num, unsigned int deg) {
	int result = 1;
	while (deg) {
		result *= num;
		deg--;
	}
	return result;
}

//
void uint_to_str(unsigned int x, char* str) {
	//char str[11] = { 0 };
	int i = 0;
	while (x) {
		str[i++] = (x % 10) + '0';
		x = x / 10;
	}
	str[i] = '\0';
	strrev(str);
}
//

void argv_parser(char* str) {
	int i = 2;
	int c = 0;
	while (str_in[i] != ' ' && str_in[i] != '\0') {
		command[c] = str_in[i];
		i++; c++;
		if (i == 40)
			return; //error
	}
	command[c] = '\0';
	i++;
	if (strcmp(command, "nsconv")) {
		c = 0;
		char arg1[34] = { 0 };
		char sarg2[11] = { 0 };
		char sarg3[11] = { 0 };
		unsigned int arg2 = 0, arg3 = 0;
		while (str_in[i] != ' ') {
			arg1[c] = str_in[i];
			i++; c++;
		}
		arg1[c] = '\0';
		i++;
		c = 0;
		while (str_in[i] != ' ') {
			sarg2[c] = str_in[i];
			i++; c++;
		}
		sarg2[c] = '\0';
		i++;
		c = 0;
		arg2 = atoi(sarg2);
		while (str_in[i] != '\0') {
			sarg3[c] = str_in[i];
			i++; c++;
		}
		sarg3[c] = '\0';
		arg3 = atoi(sarg3);
		if (!correct_nsconv(arg1, arg2, arg3))
			return;
		nsconv(arg1, arg2, arg3);
	}
	else if (strcmp(command, "posixtime")) {
		char sarg1[11] = { 0 };
		unsigned int arg1 = 0;
		c = 0;
		while (str_in[i] != '\0') {
			sarg1[c] = str_in[i];
			i++; c++;
		}
		sarg1[c] = '\0';
		if (!correct_time_pos(sarg1))
			return;
		else {
			arg1 = atoi(sarg1);
			date((unsigned int)arg1, 1970);
		}
	}
	else if (strcmp(command, "wintime")) { //test arg1 = 0!!!
		char sarg1[20] = { 0 };
		long long int arg1 = 1;
		c = 0;
		while (str_in[i] != '\0') {
			sarg1[c] = str_in[i];
			i++; c++;
		}
		sarg1[c] = '\0';
		if (!correct_time_win(sarg1))
			return;
		else {
			arg1 = atoi64(sarg1);
			date((long long int)arg1, 1601);
		}
	}
	else if (strcmp(command, "shutdown")) {
		shutdown();
	}
	else if (strcmp(command, "info")) {
		info();
	}
	else if (strcmp(command, "clear")) {
		clear();
	}
	else
		error(1);
	
}

void fill_the_string(unsigned char scan_code) {
	char symbol = 0;
	if (scan_code == 57)
		symbol = ' ';
	else if (scan_code == 4 && (caps == true || lshift == true
		|| rshift == true))
		symbol = '#';
	else if((lshift == false && rshift == false && caps == false) || 
		(caps == true && (lshift == true || rshift == true)))
		symbol = codes_table[(int)scan_code];
	if (cur_position < 81) {
		out_char(symbol);
	}
}


void on_key(unsigned char scan_code) {
	if (scan_code >= 2 && scan_code <= 11 ||
		scan_code >= 16 && scan_code <= 25 ||
		scan_code >= 30 && scan_code <= 38 ||
		scan_code >= 43 && scan_code <= 50 || scan_code == 57) {
		fill_the_string(scan_code);
		return;
	}
	else if (scan_code == 14) {
		unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
		video_buf += 2 * (VIDEO_WIDTH * cur_strnum + cur_position - 1);
		video_buf[0] = '\0';
		if (cur_position > 2 && str_in_count > 0) {
			cur_position--;
			str_in_count--;
		}
		cursor_moveto(cur_strnum, cur_position);
	}
	else if (scan_code == 42 && lshift == false) {
		lshift = true;
		return;
	}
	else if (scan_code == 54 && rshift == false) {
		rshift = true;
		return;
	}
	else if (scan_code == 58 && caps == false) {
		caps = true;
		return;
	}
	else if (scan_code == 58 && caps == true) {
		caps = false;
		return;
	}
	else if (scan_code == 170 && lshift == true) {
		lshift = false;
		return;
	}
	else if (scan_code == 182 && rshift == true) {
		rshift = false;
		return;
	}
	else if (scan_code == 4 && (lshift == true || rshift == true)) {
		fill_the_string(scan_code);
		return;
	}
	else if (scan_code == 28) {
		str_in[str_in_count] = '\0';
		cur_strnum++;
		argv_parser(str_in);
		cur_position = 0;
		cursor_moveto(cur_strnum, cur_position);
		str_in_count = 0;
		com_symbol();
	}
	//else введен ндопустимый символ
	return;


}

void keyb_process_keys() {
	// Проверка что буфер PS/2 клавиатуры не пуст (младший бит присутствует)
	if (inb(0x64) & 0x01) {
		unsigned char scan_code;
		unsigned char state;
		scan_code = inb(0x60); // Считывание символа с PS/2 клавиатуры
		if (scan_code < 128 || scan_code == 170 || scan_code == 182) // Скан-коды выше 128 - это отпускание клавиши
			on_key(scan_code);
	}
}

void cursor_moveto(unsigned int strnum, unsigned int pos) {
	unsigned short new_pos = (strnum * VIDEO_WIDTH) + pos;
	outb(CURSOR_PORT, 0x0F);
	outb(CURSOR_PORT + 1, (unsigned char)(new_pos & 0xFF));
	outb(CURSOR_PORT, 0x0E);
	outb(CURSOR_PORT + 1, (unsigned char)((new_pos >> 8) & 0xFF));
}


void out_char(unsigned char symbol) {
	unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
	video_buf += 2 * (cur_strnum * VIDEO_WIDTH + cur_position);
	video_buf[0] = symbol;
	video_buf[1] = color;
	if (cur_position < 81) {
		str_in[str_in_count] = symbol;
		cur_position++;
		str_in_count++;
		cursor_moveto(cur_strnum, cur_position);
	}
}


void out_str(const char* ptr) {
	unsigned char* video_buf = (unsigned char*) VIDEO_BUF_PTR;
	video_buf += VIDEO_WIDTH * 2 * cur_strnum;
	while (*ptr) {
		video_buf[0] = (unsigned char)*ptr; // Символ (код)
		video_buf[1] = color; // Цвет символа и фона
		video_buf += 2;
		ptr++;
	}
	cur_strnum++;
	cur_position = 0;
	if (cur_strnum > 23)
		clear();
	cursor_moveto(cur_strnum, cur_position);
	//scroll_screen
}


void load_color() {
	char boot_color;
	__asm
	{
		mov boot_color, bl;
	}
	if (boot_color == 1) color = 0x07;
	else if (boot_color == 2) color = 0x0F;
	else if (boot_color == 3) color = 0x0E;
	else if (boot_color == 4) color = 0x01;
	else if (boot_color == 5) color = 0x04;
	else if (boot_color == 6) color = 0x02;
}

void welcome_string() {
	const char* hello = "Welcome to ConvertOS!";
	load_color();
	out_str(hello);
}

void com_symbol() {
	out_char('#');
	out_char(' ');
}

void error(int error_num) {
	//clear_bool = false; info_bool = false;
	//nsconv_bool = false; wintime_bool = false; posixtime_bool = false; szconv_bool = false;
	//memset(string_in);
	const char* error_in = { 0 };
	if (error_num == 1)
		error_in = "Error: invalid command";
	else if (error_num == 2
		)error_in = "Error: overflow";
	else if (error_num == 3
		)error_in = "Error: the source number system does not support the entered number";
	else if (error_num == 4)
		error_in = "Error: incorrect number system (possible number systems from 2 to 36)";
	else if (error_num == 5)
		error_in = "Error: invalid number input";
	else if (error_num == 6)
		error_in = "Error: invalid input";
	out_str(error_in);
}


void info() {
	out_str("ConvertOS:");
	out_str("Developer: Aksenov Danil, group number: 5131001/20001");
	out_str("Assembler Translator: FASM, syntax: Intel");
	out_str("Kernel Compiler: Microsodt C Compiler");
	if (color == 0x07) out_str("OS Console Font Color: Gray");
	else if (color == 0x0F) out_str("OS Console Font Color: White");
	else if (color == 0x0E) out_str("OS Console Font Color: Yellow");
	else if (color == 0x01) out_str("OS Console Font Color: Blue");
	else if (color == 0x04) out_str("OS Console Font Color: Red");
	else if (color == 0x02) out_str("OS Console Font Color: Green");

}

void clear() {
	unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
	for (int i = 0; i < VIDEO_WIDTH * 25; i++) {
		*(video_buf + i * 2) = '\0';
	}
	cur_strnum = 0;
	cur_position = 0;
	cursor_moveto(cur_strnum, cur_position);
}

static inline void outw(unsigned short port, unsigned short data) {
	__asm {
		mov dx, port
		mov ax, data
		out dx, ax
	}
}

void shutdown() {
	outw(0x604, 0x2000);
}

void date_to_string(long long int day, unsigned int month,
	unsigned int year, unsigned int hour, unsigned int min, 
	unsigned int sec) {
	int count = 0;
	char date_string[20] = { 0 };
	int date[6] = { 0 };
	char year_str[5] = { 0 };
	char some_date[3] = { 0 };
	date[0] = day, date[1] = month, date[2] = year; 
	date[3] = hour, date[4] = min, date[5] = sec;
	for (int i = 0; i < 6; i++) {
		if (i == 2) {
			uint_to_str(year, year_str);
			for (int j = 0; j < 4; j++) {
				date_string[count] = year_str[j];
				count++;
			}
			year_str[4] = '\0';
		}
		else {
			uint_to_str(date[i], some_date);
			if (date[i] == 0) { //фикс :00
				date_string[count] = '0';
				count++;
				date_string[count] = '0';
				count++;
			}
			else if (date[i] < 10) {
				date_string[count] = '0';
				count++;
				date_string[count] = some_date[0];
				count++;
			}
			else {
				date_string[count] = some_date[0];
				count++;
				date_string[count] = some_date[1];
				count++;
			}
			some_date[2] = '\0';
		}
		if (count == 2 || count == 5)
			date_string[count] = '.';
		if (count == 13 || count == 16)
			date_string[count] = ':';
		if (count == 10)
			date_string[count] = ' ';
		count++;
	}
	date_string[count] = '\0';
	out_str(date_string);
}


void date(long long int number, int year) {
	long long int count_in_sec = 1; // long long == 0 BANNED
	unsigned int sec = 0;
	if (number == 0) {
		date_to_string(1, 1, year, 0, 0, 0);
	}

	if (year == 1601) {
		number /= 10000000;
		sec = number % 60;
		number /= 60;
	}
	else {
		sec = number % 60;
		number /= 60;
	}
	unsigned int min = number % 60;
	number /= 60;
	unsigned int hour = number % 24;
	number /= 24;
	unsigned int years = year;
	int tmp = 0;

	while (number > 365) {
		if (years % 4 == 0 && years % 100 != 0 || years % 400 == 0)
			number -= 366;
		else
			number -= 365;
		years++;
	}
	unsigned int days = number;
	unsigned int month = 1;
	if (days > 31) { //jan
		month++;
		days -= 31;
	}
	else {
		date_to_string(days + 1, month, years, hour, min, sec);
		return;
	}
	if (years % 4 == 0) { //feb
		if (days > 29) {
			month++;
			days -= 29;
		}
		else {
			date_to_string(days + 1, month, years, hour, min, sec);
			return;
		}
	}
	else {
		if (days > 28) {
			month++;
			days -= 28;
		}
		else {
			date_to_string(days + 1, month, years, hour, min, sec);
			return;
		}
	}
	if (days > 31) { //march
		month++;
		days -= 31;
	}
	else {
		date_to_string(days + 1, month, years, hour, min, sec);
		return;
	}
	if (days > 30) { //april
		month++;
		days -= 30;
	}
	else {
		date_to_string(days + 1, month, years, hour, min, sec);
		return;
	}
	if (days > 31) { //may
		month++;
		days -= 31;
	}
	else {
		date_to_string(days + 1, month, years, hour, min, sec);
		return;
	}
	if (days > 30) { //june
		month++;
		days -= 30;
	}
	else
		date_to_string(days + 1, month, years, hour, min, sec);
	if (days > 31) { //july
		month++;
		days -= 31;
	}
	else {
		date_to_string(days + 1, month, years, hour, min, sec);
		return;
	}
	if (days > 31) { //aug
		month++;
		days -= 31;
	}
	else {
		date_to_string(days + 1, month, years, hour, min, sec);
		return;
	}
	if (days > 30) { //sep
		month++;
		days -= 3;
	}
	else {
		date_to_string(days + 1, month, years, hour, min, sec);
		return;
	}
	if (days > 31) { //oct
		month++;
		days -= 31;
	}
	else {
		date_to_string(days + 1, month, years, hour, min, sec);
		return;
	}
	if (days > 30) { //nov
		month++;
		days -= 30;
	}
	else {
		date_to_string(days + 1, month, years, hour, min, sec);
		return;
	}
	if (days > 31) { //dec
		month++;
		days -= 31;
	}
	date_to_string(days + 1, month, years, hour, min, sec);
}

unsigned int to_10ss(char* nx, unsigned int ss) {
	unsigned int ss10_num = 0;
	int k = 0;
	strrev(nx);
	while (nx[k] != '\0') {
		int i = 0;
		while (nx[k] != sys_symbols[i]) {
			i++;
		}
		ss10_num += i * pow(ss, k);
		k++;
	}
	return ss10_num;
}

void nsconv(char* arg1, int arg2, int arg3) {
	unsigned int ss10_number = to_10ss(arg1, arg2);
	char result[12] = { 0 };
	int i = 0;
	while (ss10_number) {
		result[i] = sys_symbols[ss10_number % arg3];
		ss10_number /= arg3;
		i++;
	}
	result[i] = '\0';
	strrev(result);
	out_str(result);
}

const char* g_test = "This is test string.";
extern "C" int kmain() {
	
	welcome_string();
	intr_init();
	keyb_init();
	intr_start();
	intr_enable();
	com_symbol();
	// Бесконечный цикл
	while (1)
	{
		__asm hlt;
	}
	return 0;
}


use16
org 0x7C00
start:
        mov ax, cs ; Сохранение адреса сегмента кода в ax
        mov ds, ax ; Сохранение этого адреса как начало сегмента данных
        mov ss, ax ; И сегмента стека
        mov sp, start ; Сохранение адреса стека как адрес первой инструкции этого кода. Стек будет расти вверх и не перекроет код.
        mov ah, 0x0e ; В ah номер функции BIOS: 0x0e - вывод символа на активную видео страницу (эмуляция телетайпа)
       ; call clear
read: ;чтение ядра
        mov ax,0x1100
        mov es,ax    ; запись любого числа напрямую в сегментные регистры запрещена! Поэтому вместо одной команды приходится выполнять две
        mov bx, 0x00 ; Адрес, откуда загружается ядро
        mov dl, 0x01 ; Номер диска (носителя)
        mov dh, 0x00 ; Номер головки
        mov cl, 0x02 ; Номер сектора
        mov ch, 0x00 ; Номер цилиндр
        mov al, 0x10 ; Количество секторов
        mov ah, 0x02 ; Функция 0x02 прерывания 0х13 (считывание заданного количества секторов с диска в память)
        int 0x13 ; Прерывание на дисковый ввод/вывод

        mov ax, 0x1300
        mov es, ax
        mov bx, 0x00 ; Адрес, откуда загружается ядро
        mov dl, 0x01 ; Номер диска (носителя)
        mov dh, 0x00 ; Номер головки
        mov cl, 0x12 ; Номер сектора
        mov ch, 0x00 ; Номер цилиндр
        mov al, 0x07 ; Количество секторов
        mov ah, 0x02 ; Функция 0x02 прерывания 0х13 (считывание заданного количества секторов с диска в память)
        int 0x13 ; Прерывание на дисковый ввод/вывод

        ;mov edx, 0x00
        ;mov ch, 0x00
        xor ch, ch
        mov cl, 1
;cl - текущий выбранный цвет
;ch - текущая строка цвета для печати
;edx - счетчик строк
menu:
        call clear
        mov edi, 0xb8000 ;Начало области памяти, выделенной под видеоадаптер
        mov edx, 0x00 ;Счетчик
        mov esi, loading_string ; Загрузка адреса строки
        call choice_color
        mov ch, 1 ;Счетчик цвета для печати
        call new_line
        mov esi, gray
        call choice_color
        call new_line
        mov esi, white
        call choice_color
        call new_line
        mov esi, yellow
        call choice_color
        call new_line
        mov esi, blue
        call choice_color
        call new_line
        mov esi, red
        call choice_color
        call new_line
        mov esi, green
        call choice_color
key_input:
        mov ah, 0x00 ;ожидание нажатия и считывание нажатой пользователем клавиши
        int 0x16
        cmp ah, 0x48 ;вверх
        je move_up
        cmp ah, 0x50 ;вниз
        je move_down
        cmp ah, 0x1C ;enter
        je load_kernel ;загрузка ядра
        jmp key_input ;если иная клавиша, выполняется повторное считывание
move_up:
        cmp cl, 1 ;Если текущий цвет начальный, то следующий цвет будет последним
        je overflow_above
        dec cl ;Иначе декрементируем
        jmp menu ;Выводим меню
move_down:
        cmp cl, 6 ;Если текущий цвет последний, то следующий цвет будет начальным
        je overflow_below
        inc cl ;Иначе инкрементируем
        jmp menu
overflow_above: ;переход курсора сверху вниз при переполнении
        mov cl, 6
        jmp menu
overflow_below: ;переход курсора снизу вверх при переполнении
        mov cl, 1
        jmp menu
clear: ;Очистка экрана
       mov ax, 3
       int 0x10
       ret
new_line:
        inc edx ;Номер следующей строки
        mov ebx, edx ;Копируем
        imul ebx, 160 ;Умножаем на 160 (80 символов в строке, по 2 байта на каждый)
        mov edi, 0xb8000
        add edi, ebx ;Прибавляем к началу памяти
        ret
loading_string:
        db "Select console color:", 0  ;получение строки
gray:
        db "Gray", 0
white:
        db "White", 0
yellow:
        db "Yellow", 0
blue:
        db "Blue", 0
red:
        db "Red", 0
green:
        db "Green", 0
;Функции, записывающие цвет текущего выбора
gray_color:
        mov ah, 0x07
        jmp video_puts
white_color:
        mov ah, 0x0f
        jmp video_puts
yellow_color:
        mov ah, 0x0e
        jmp video_puts
blue_color:
        mov ah, 0x01
        jmp video_puts
red_color:
        mov ah, 0x04
        jmp video_puts
green_color:
        mov ah, 0x02
        jmp video_puts
choice_color:
        mov ah, 0x08 ;Дефолтный цвет (темно-серый)
        cmp ch,cl ;Если текущий цвет и текущий цвет для печати не равны, то выводим темно-серым
        jne video_puts
        cmp cl, 1 ;Иначе выбираем для него цвет
        je gray_color
        cmp cl, 2
        je white_color
        cmp cl, 3
        je yellow_color
        cmp cl, 4
        je blue_color
        cmp cl, 5
        je red_color
        cmp cl, 6
        je green_color
        jmp video_puts
video_puts:
        mov al, [esi] ; Загрузка символа стоки из памяти
        test al, al   ; Проверка, на конец строки (0)
        jz color_inc     ; Если 0, то цвет инкрементируется (строка закончилась)
        mov [edi], al ; Скопировать символ в буфер видеопамяти
        mov [edi+1], ah ; Сохранить цветовой атрибут символа
        add edi, 2 ; Переход к след. символу в буфере видеопамяти
        add esi, 1 ; Переход к след. символу в строке
        jmp video_puts
color_inc:
        inc ch ;Инкремент цвета
        ret
load_kernel:
        xor ebx, ebx
        mov bl, cl
        call clear
        cli ; Отключение прерываний
        lgdt [gdt_info] ; Загрузка размера и адреса таблицы дескрипторов
        in al, 0x92
        or al, 2 ; Включение адресной линии А20
        out 0x92, al
        mov eax, cr0
        or al, 1 ; Установка бита PE регистра CR0 - процессор перейдет в защищенный режим
        mov cr0, eax
        jmp 0x8:protected_mode
gdt:
        ; Нулевой дескриптор
        db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ; Сегмент кода: base=0, size=4Gb, P=1, DPL=0, S=1(user),
        ; Type=1(code), Access=00A, G=1, B=32bit
        db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
        ; Сегмент данных: base=0, size=4Gb, P=1, DPL=0, S=1(user),
        ; Type=0(data), Access=0W0, G=1, B=32bit
        db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00
        gdt_info: ; Данные о таблице GDT (размер, положение в памяти)
        dw gdt_info - gdt ; Размер таблицы (2 байта)
        dw gdt, 0 ; 32-битный физический адрес таблицы.
use32
protected_mode:
        ; Загрузка селекторов сегментов для стека и данных в регистры
        mov ax, 0x10 ; Используется дескриптор с номером 2 в GDT
        mov es, ax
        mov ds, ax
        mov ss, ax
        ; Передача управления загруженному ядру
        call 0x11000 ; Адрес равен адресу загрузки в случае если ядро скомпилировано в"плоский" код
        ; Внимание! Сектор будет считаться загрузочным, если содержит в конце своих 512 байтов два следующих байта: 0x55 и 0xAA
        times (512 - ($ - start) - 2) db 0 ; Заполнение нулями до границы 512 - 2 текущей точки
        db 0x55, 0xAA ; 2 необходимых байта чтобы сектор считался загрузочным
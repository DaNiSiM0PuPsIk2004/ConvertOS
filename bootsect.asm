use16
org 0x7C00
start:
        mov ax, cs ; ���������� ������ �������� ���� � ax
        mov ds, ax ; ���������� ����� ������ ��� ������ �������� ������
        mov ss, ax ; � �������� �����
        mov sp, start ; ���������� ������ ����� ��� ����� ������ ���������� ����� ����. ���� ����� ����� ����� � �� ��������� ���.
        mov ah, 0x0e ; � ah ����� ������� BIOS: 0x0e - ����� ������� �� �������� ����� �������� (�������� ���������)
       ; call clear
read: ;������ ����
        mov ax,0x1100
        mov es,ax    ; ������ ������ ����� �������� � ���������� �������� ���������! ������� ������ ����� ������� ���������� ��������� ���
        mov bx, 0x00 ; �����, ������ ����������� ����
        mov dl, 0x01 ; ����� ����� (��������)
        mov dh, 0x00 ; ����� �������
        mov cl, 0x02 ; ����� �������
        mov ch, 0x00 ; ����� �������
        mov al, 0x10 ; ���������� ��������
        mov ah, 0x02 ; ������� 0x02 ���������� 0�13 (���������� ��������� ���������� �������� � ����� � ������)
        int 0x13 ; ���������� �� �������� ����/�����

        mov ax, 0x1300
        mov es, ax
        mov bx, 0x00 ; �����, ������ ����������� ����
        mov dl, 0x01 ; ����� ����� (��������)
        mov dh, 0x00 ; ����� �������
        mov cl, 0x12 ; ����� �������
        mov ch, 0x00 ; ����� �������
        mov al, 0x07 ; ���������� ��������
        mov ah, 0x02 ; ������� 0x02 ���������� 0�13 (���������� ��������� ���������� �������� � ����� � ������)
        int 0x13 ; ���������� �� �������� ����/�����

        ;mov edx, 0x00
        ;mov ch, 0x00
        xor ch, ch
        mov cl, 1
;cl - ������� ��������� ����
;ch - ������� ������ ����� ��� ������
;edx - ������� �����
menu:
        call clear
        mov edi, 0xb8000 ;������ ������� ������, ���������� ��� ������������
        mov edx, 0x00 ;�������
        mov esi, loading_string ; �������� ������ ������
        call choice_color
        mov ch, 1 ;������� ����� ��� ������
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
        mov ah, 0x00 ;�������� ������� � ���������� ������� ������������� �������
        int 0x16
        cmp ah, 0x48 ;�����
        je move_up
        cmp ah, 0x50 ;����
        je move_down
        cmp ah, 0x1C ;enter
        je load_kernel ;�������� ����
        jmp key_input ;���� ���� �������, ����������� ��������� ����������
move_up:
        cmp cl, 1 ;���� ������� ���� ���������, �� ��������� ���� ����� ���������
        je overflow_above
        dec cl ;����� ��������������
        jmp menu ;������� ����
move_down:
        cmp cl, 6 ;���� ������� ���� ���������, �� ��������� ���� ����� ���������
        je overflow_below
        inc cl ;����� ��������������
        jmp menu
overflow_above: ;������� ������� ������ ���� ��� ������������
        mov cl, 6
        jmp menu
overflow_below: ;������� ������� ����� ����� ��� ������������
        mov cl, 1
        jmp menu
clear: ;������� ������
       mov ax, 3
       int 0x10
       ret
new_line:
        inc edx ;����� ��������� ������
        mov ebx, edx ;��������
        imul ebx, 160 ;�������� �� 160 (80 �������� � ������, �� 2 ����� �� ������)
        mov edi, 0xb8000
        add edi, ebx ;���������� � ������ ������
        ret
loading_string:
        db "Select console color:", 0  ;��������� ������
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
;�������, ������������ ���� �������� ������
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
        mov ah, 0x08 ;��������� ���� (�����-�����)
        cmp ch,cl ;���� ������� ���� � ������� ���� ��� ������ �� �����, �� ������� �����-�����
        jne video_puts
        cmp cl, 1 ;����� �������� ��� ���� ����
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
        mov al, [esi] ; �������� ������� ����� �� ������
        test al, al   ; ��������, �� ����� ������ (0)
        jz color_inc     ; ���� 0, �� ���� ���������������� (������ �����������)
        mov [edi], al ; ����������� ������ � ����� �����������
        mov [edi+1], ah ; ��������� �������� ������� �������
        add edi, 2 ; ������� � ����. ������� � ������ �����������
        add esi, 1 ; ������� � ����. ������� � ������
        jmp video_puts
color_inc:
        inc ch ;��������� �����
        ret
load_kernel:
        xor ebx, ebx
        mov bl, cl
        call clear
        cli ; ���������� ����������
        lgdt [gdt_info] ; �������� ������� � ������ ������� ������������
        in al, 0x92
        or al, 2 ; ��������� �������� ����� �20
        out 0x92, al
        mov eax, cr0
        or al, 1 ; ��������� ���� PE �������� CR0 - ��������� �������� � ���������� �����
        mov cr0, eax
        jmp 0x8:protected_mode
gdt:
        ; ������� ����������
        db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ; ������� ����: base=0, size=4Gb, P=1, DPL=0, S=1(user),
        ; Type=1(code), Access=00A, G=1, B=32bit
        db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
        ; ������� ������: base=0, size=4Gb, P=1, DPL=0, S=1(user),
        ; Type=0(data), Access=0W0, G=1, B=32bit
        db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00
        gdt_info: ; ������ � ������� GDT (������, ��������� � ������)
        dw gdt_info - gdt ; ������ ������� (2 �����)
        dw gdt, 0 ; 32-������ ���������� ����� �������.
use32
protected_mode:
        ; �������� ���������� ��������� ��� ����� � ������ � ��������
        mov ax, 0x10 ; ������������ ���������� � ������� 2 � GDT
        mov es, ax
        mov ds, ax
        mov ss, ax
        ; �������� ���������� ������������ ����
        call 0x11000 ; ����� ����� ������ �������� � ������ ���� ���� �������������� �"�������" ���
        ; ��������! ������ ����� ��������� �����������, ���� �������� � ����� ����� 512 ������ ��� ��������� �����: 0x55 � 0xAA
        times (512 - ($ - start) - 2) db 0 ; ���������� ������ �� ������� 512 - 2 ������� �����
        db 0x55, 0xAA ; 2 ����������� ����� ����� ������ �������� �����������
/*
 * 	startup.c
 *
 */
//#include <decl.h>




//Define delen


#define UP 0x2000
#define DOWN 0x0020
#define RIGHT 0x0400
#define LEFT 0x0100

//VEKTOR TABELLEN
#define VTOR ((volatile unsigned int*) 0xE000ED08)


//SYSTICK
#define SYSTICK 0xE000E010
#define STK_CTRL ((volatile unsigned int*) SYSTICK)
#define STK_LOAD ((volatile unsigned int*) (SYSTICK + 0x4))
#define STK_VAL ((volatile unsigned int*) (SYSTICK + 0x8))
#define SYSTICK_IRQVEK ((void (**) (void)) 0x2001C03C)


//PORT D
#define	PORT_D_BASE	0x40020C00
#define	GPIO_D_MODER 	((volatile unsigned int *)	(PORT_D_BASE))  
#define	GPIO_D_OTYPER 	((volatile unsigned short *)	(PORT_D_BASE+0x4))  
#define	GPIO_D_PUPDR 	((volatile unsigned int *)	(PORT_D_BASE+0xC))  
#define	GPIO_D_HIGH_IDR	((volatile unsigned char *)	(PORT_D_BASE+0x11))  
#define	GPIO_D_LOW_ODR 	((volatile unsigned char *)	(PORT_D_BASE+0x14))
#define	GPIO_D_HIGH_ODR 	((volatile unsigned char *)	(PORT_D_BASE+0x15))

//TIM6 define
#define TIM6_CR1 ((volatile unsigned int*) 0x40001000)
#define TIM6_DEIR ((volatile unsigned int*) 0x4000100C)
#define TIM6_SR ((volatile unsigned int*) 0x40001010)
#define TIM6_PSC ((volatile unsigned int*) 0x40001028)
#define TIM6_ARR ((volatile unsigned int*) 0x4000102C)
#define TIM6_CNT ((volatile unsigned int*) 0x40001024)
#define NVIC_ISER1 ((volatile unsigned int*) 0xE000E104)
#define NVIC_ICER1 ((volatile unsigned int*) 0xE000E184)
#define TIM6_IRQVEC ((void (**) (void)) 0x2001C118)
#define NVIC_TIM6_IRQ_BPOS (1 << 22)
#define UIE (1)
#define CEN (1)
#define UIF (1)
__attribute__((naked)) __attribute__((section (".start_section")) )

void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}






void app_init(void){
	*VTOR = 0x2001C000;
	*GPIO_D_MODER = 0x55005555;
	*GPIO_D_OTYPER = 0;
	*GPIO_D_PUPDR = 0x00AA0000;
	*TIM6_CR1 &= ~CEN;
	*TIM6_DEIR |= (1 << 1);
	*TIM6_ARR = 128;
	*TIM6_CR1 |= CEN;
	*STK_CTRL = 0;
	*STK_LOAD = 167;
	*STK_CTRL = 5;
	*STK_VAL = 0;
}



__attribute__((naked))
void graphic_initalize(void){
	__asm volatile (" .HWORD 0xDFF0\n");
	__asm volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_clear_screen(void){
	__asm volatile (" .HWORD 0xDFF1\n");
	__asm volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_set(int x, int y){
	__asm volatile (" .HWORD 0xDFF2\n");
	__asm volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_clear(int x, int y){
	__asm volatile (" .HWORD 0xDFF3\n");
	__asm volatile (" BX LR\n");
}


typedef struct{
	signed char x, y;
}POINT,*PPOINT;


typedef struct{
	int numPoints;
	int sizex;
	int sizey;
	POINT px[9];
}GEOMETRY, *PGEOMETRY;



typedef struct bodypart{
	PGEOMETRY geo;
	signed int dirx, diry;
	signed int posx, posy;
	void (*draw_bodypart) (struct bodypart);
	void (*clear_bodypart) (struct bodypart);
	void (*move_bodypart) (struct bodypart*);
	void (*set_object_speed) (struct bodypart* , int , int);
}BODYPART, *PBODYPART;

typedef struct {
	int numberofparts;
	PBODYPART bodyparts[100];
}SNAKE, *PSNAKE;

typedef struct apple{
	PGEOMETRY geo;
	int on;
	int posx, posy;
	void (*appear) (struct apple*);
	void (*draw_apple) (struct apple*);
	void (*clear_apple) (struct apple*);
}APPLE, *PAPPLE;

//Funktioners headers
void app_init(void);
void draw_object (BODYPART o);
void clear_object (BODYPART o);
void set_object_speed (PBODYPART o, int x, int y);
unsigned short keyb_alt_ctrl(void);
void activateRow (int row);
void move_head (PBODYPART bodyPart);
void move_snake (PSNAKE snake);
void draw_apple (APPLE apple);
void apple_appear (PAPPLE apple);
void clear_apple (APPLE apple);
int snake_eat_apple (PSNAKE snake, PAPPLE apple);
void add_apple(PSNAKE snake, PAPPLE apple);
void delay_mikro(int us);
void move_body (PSNAKE snake);
void snake_hit_wall (PSNAKE snake);
void snake_bite_snake (PSNAKE snake);
void draw_snake (PSNAKE snake);
void clear_snake (PSNAKE snake);

//Variable deklaration

/*
extern GEOMETRY headGeometry;
extern GEOMETRY kotaGeometry;
extern BODYPART head;
extern BODYPART kota;
extern SNAKE snake;
*/


void delay_mikro (int us){
	for (int i = 0; i < us; i++){
		while(*STK_CTRL & 0x1000 == 0);
		*STK_CTRL = 0;
	}
}



static GEOMETRY bodyPartGeometry = {
	9, 
	3, 3,
	{{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2}}
};

static BODYPART bodyPart = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_head,
	set_object_speed
};
static BODYPART bodyPart1 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart2 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart3 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart4 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart5 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart6 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart7 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart8 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart9 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart10 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart11 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart12 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart13 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart14 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart15 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart16 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart17 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static BODYPART bodyPart18 = {
	&bodyPartGeometry,
	0, 3,
	64, 32,
	draw_object,
	clear_object,
	move_body,
	set_object_speed
};
static GEOMETRY appleGeometry = {
	9,
	3, 3,
	{{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}, {2, 0}, {2, 1}, {2, 2}}
};

PBODYPART bb[100];
SNAKE snake = {
	1,
	{&bodyPart, &bodyPart1, &bodyPart2, &bodyPart3, &bodyPart4, &bodyPart5, &bodyPart6, &bodyPart7, &bodyPart8, &bodyPart9
	, &bodyPart10, &bodyPart11, &bodyPart12, &bodyPart13, &bodyPart14, &bodyPart15, &bodyPart16, &bodyPart17, &bodyPart18}
};


static APPLE adams_apple = {
	&appleGeometry,
	0,
	0, 0,
	apple_appear,
	draw_apple,
	clear_apple
};

void activateRow(int row){
		switch (row){
		case 1: *GPIO_D_HIGH_ODR = 0x10; break;
		case 2: *GPIO_D_HIGH_ODR = 0x20; break;
		case 3: *GPIO_D_HIGH_ODR = 0x40; break;
		case 4: *GPIO_D_HIGH_ODR = 0x80; break;
		default: *GPIO_D_HIGH_ODR = 0; break;
	}
}

unsigned short keyb_alt_ctrl(void){
	short keyb_status = 0;
	short x = 0;
	for (int i = 4; i > 0; i--){
		activateRow(i);
		x = (*GPIO_D_HIGH_IDR);
		x = (x << (4 - i)*4);
		keyb_status =  keyb_status | x;
	}return (keyb_status);
}


void clear_object(BODYPART b){
	for (int i = 0; i < b.geo -> numPoints; i++){
		char x = b.geo -> px[i].x + b.posx;
		char y = b.geo -> px[i].y + b.posy;
		graphic_pixel_clear(x, y);
	}
};

void draw_object(BODYPART b){
	for (int i = 0; i < b.geo -> numPoints; i++){
		char x = b.geo -> px[i].x + b.posx;
		char y = b.geo -> px[i].y + b.posy;
		graphic_pixel_set(x, y);
	}
};


void set_object_speed(PBODYPART pb, int x, int y){
	pb->dirx = x;
	pb->diry = y;
};


void draw_snake (PSNAKE snake){
	for (int i = 0; i < snake->numberofparts; i++){
		draw_object(*snake->bodyparts[i]);
	}
}


void clear_snake (PSNAKE snake){
	for (int i = 0; i < snake->numberofparts; i++){
		clear_object(*snake->bodyparts[i]);
	}
}


void move_head (PBODYPART head){
	head->posx = head->posx + head->dirx;
	head->posy = head->posy + head->diry;
}

void move_body (PSNAKE snake){
	for (int i = snake->numberofparts - 1; i > 0; i--){
		snake->bodyparts[i]->posx = snake->bodyparts[i - 1]->posx;
		snake->bodyparts[i]->posy = snake->bodyparts[i - 1]->posy;
	}
}

void set_object_direction (PSNAKE snake){
	unsigned short us = keyb_alt_ctrl();
	if ((snake->bodyparts[0]->dirx == 1 && (us & LEFT)) || (snake->bodyparts[0]->dirx == -1 && (us & RIGHT)) ||
		(snake->bodyparts[0]->diry == 1 && (us & UP)) || (snake->bodyparts[0]->diry == -1 && (us & DOWN))){
			;
		}else{
			switch (us){
				case UP:				 
					snake->bodyparts[0]->dirx = 0;
					snake->bodyparts[0]->diry = -3;
					break;
				case DOWN: 
					snake->bodyparts[0]->dirx = 0;
					snake->bodyparts[0]->diry = 3;
					break;
				case RIGHT: 
					snake->bodyparts[0]->dirx = 3;
					snake->bodyparts[0]->diry = 0;
				    break;
				case LEFT: 
					snake->bodyparts[0]->dirx = -3;
					snake->bodyparts[0]->diry = 0;
					break;
			default:
					break;
			}
		}
}


void move_snake(PSNAKE snake){
	clear_snake(snake);
	set_object_direction (snake);
	move_body(snake);
	move_head(snake->bodyparts[0]);
	draw_snake(snake);
}


void draw_apple (APPLE apple){
	for (int i = 0; i < apple.geo->numPoints; i++){
		int x = apple.posx + apple.geo->px[i].x;
		int y = apple.posy + apple.geo->px[i].y;
		graphic_pixel_set(x, y);
	}
}

void clear_apple (APPLE apple){
	for (int i = 0; i < apple.geo->numPoints; i++){
		int x = apple.posx + apple.geo->px[i].x;
		int y = apple.posy + apple.geo->px[i].y;
		graphic_pixel_clear(x, y);
	}
}

void apple_appear (PAPPLE apple){
	if (apple->on == 1){
		apple->on = 1;
	}
	else{
		int x = 1;
		while(x){
			if ((*TIM6_CNT > 6) && (*TIM6_CNT <120)){
				apple->on = 1;
				apple->posx = *TIM6_CNT;
				apple->posy = (*TIM6_CNT) / 2;
				draw_apple(*apple);
				x = 0;
			}
		}
	}
}


int snake_eat_apple(PSNAKE snake, PAPPLE apple){
	if (((snake->bodyparts[0]->posx  >= apple->posx - 1) && (snake->bodyparts[0]->posx <= apple->posx + apple->geo->sizex + 1)) &&
		((snake->bodyparts[0]->posy >= apple->posy - 1) &&  (snake->bodyparts[0]->posy <= apple->posy + apple->geo->sizey + 1)) ||
		((snake->bodyparts[0]->posx + snake->bodyparts[0]->geo->sizex >= apple->posx) && (snake->bodyparts[0]->posx <= apple->posx) &&
		(snake->bodyparts[0]->posy + snake->bodyparts[0]->geo->sizey >= apple->posy) && (snake->bodyparts[0]->posy <= apple->posy))){
			return 1;
		}else{
			return 0;
		}
}


void add_apple (PSNAKE snake, PAPPLE apple){
	snake->numberofparts++;
	apple->on = 0;
}


void snake_hit_wall (PSNAKE snake){
	if (((snake->bodyparts[0]->posx + snake->bodyparts[0]->geo->sizex) >= 128) || (snake->bodyparts[0]->posx <= 0) ||
		(snake->bodyparts[0]->posy + snake->bodyparts[0]->geo->sizey >= 64) || (snake->bodyparts[0]->posy <= 0)){
			while (1){
				draw_snake(snake);
				clear_snake(snake);
			}
		}
}


void snake_bite_snake (PSNAKE snake){
	if (snake->numberofparts >= 5){
		for (int i = 5; i < snake->numberofparts; i++){
			if (((snake->bodyparts[0]->posx  >= snake->bodyparts[i]->posx) && (snake->bodyparts[0]->posx <= snake->bodyparts[i]->posx + snake->bodyparts[i]->geo->sizex)) &&
				((snake->bodyparts[0]->posy >= snake->bodyparts[i]->posy) &&  (snake->bodyparts[0]->posy <= snake->bodyparts[i]->posy + snake->bodyparts[i]->geo->sizey)) ||
				((snake->bodyparts[0]->posx + snake->bodyparts[0]->geo->sizex >= snake->bodyparts[i]->posx) && (snake->bodyparts[0]->posx <= snake->bodyparts[i]->posx)) &&
				((snake->bodyparts[0]->posy + snake->bodyparts[0]->geo->sizey >= snake->bodyparts[i]->posy) && (snake->bodyparts[0]->posy <= snake->bodyparts[i]->posy))){
					while (1){
						draw_snake(snake);
						clear_snake(snake);
					}
				}
		}
	}
}

void main(void){
	app_init();
	graphic_clear_screen();
	graphic_initalize();
	while (1){
		move_snake(&snake);
		delay_mikro(1000);
		apple_appear(&adams_apple);
		snake_bite_snake(&snake);
		snake_hit_wall(&snake);
		if (snake_eat_apple(&snake, &adams_apple) == 1){
			add_apple(&snake, &adams_apple);
			clear_apple(adams_apple);
		}
	}
}



/**
 * Project: IMP-PROJECT: Snake Game
 *
 * @file main.c
 *
 * @brief Implement functionality of Snake Game for fitkitv3.0
 *
 * @author Zdenek Lapes <lapes.zdenek@gmail.com> (xlapes02)
 */

#include "MK60D10.h"
#include "core_cm4.h"

#include "stdio.h"
#include "stdbool.h"

/******************************************************************************/
/*                                  MACROS                                    */
/******************************************************************************/
/* Macros for bit-level registers manipulation */
#define GPIO_PIN_MASK 0x1Fu // unsigned int (0000 0000 | 0000 0000 | 0000 0001 | 1111 1111)
#define GPIO_PIN(x) (((1) << (x & GPIO_PIN_MASK)))

/* Constants specifying delay loop duration */
#define tdelay1 100000
#define tdelay2 2

// Columns
#define A0_OFSET 0
#define A1_OFSET 1
#define A2_OFSET 2
#define A3_OFSET 3
//
#define A0_PIN 8
#define A1_PIN 10
#define A2_PIN 6
#define A3_PIN 11
//
#define A0 (1 << A0_PIN)
#define A1 (1 << A1_PIN)
#define A2 (1 << A2_PIN)
#define A3 (1 << A3_PIN)

// Rows
//
#define R0_OFSET 0
#define R1_OFSET 1
#define R2_OFSET 2
#define R3_OFSET 3
#define R4_OFSET 4
#define R5_OFSET 5
#define R6_OFSET 6
#define R7_OFSET 7
//
#define R0_PIN 26
#define R1_PIN 24
#define R2_PIN 9
#define R3_PIN 25
#define R4_PIN 28
#define R5_PIN 7
#define R6_PIN 27
#define R7_PIN 29
//
#define R0 (1 << R0_PIN)
#define R1 (1 << R1_PIN)
#define R2 (1 << R2_PIN)
#define R3 (1 << R3_PIN)
#define R4 (1 << R4_PIN)
#define R5 (1 << R5_PIN)
#define R6 (1 << R6_PIN)
#define R7 (1 << R7_PIN)

// Buttons
#define BTN_SW2         0x400     // Port E, bit 10
#define BTN_SW3         0x1000    // Port E, bit 12
#define BTN_SW4         0x8000000 // Port E, bit 27
#define BTN_SW5         0x4000000 // Port E, bit 26
#define BTN_SW6         0x800     // Port E, bit 11
#define BTN_IRQC_MASK   0x080000
#define BTNs_ALL_MASK   (BTN_SW2 | BTN_SW3 | BTN_SW4 | BTN_SW5 | BTN_SW6)

// FITKIT Clicked Buttons
#define IS_CLICK_RIGHT      (PORTE->ISFR & BTN_SW2)
#define IS_CLICK_LEFT       (PORTE->ISFR & BTN_SW4)
#define IS_CLICK_DOWN       (PORTE->ISFR & BTN_SW3)
#define IS_CLICK_UP         (PORTE->ISFR & BTN_SW5)
#define IS_CLICK_START_STOP (PORTE->ISFR & BTN_SW6)

// LED
#define ENABLE_LED_WRITE PTE->PDOR &= ~GPIO_PDOR_PDO(GPIO_PIN(28));
#define DISABLE_LED_WRITE PTE->PDOR &= 0xFFFFFFFF;

// TIMER
#define TIMER_CHANGE_OFSET(i) ((i)%(snake.speed) == 0)

/******************************************************************************/
/*                                  ENUMS                                     */
/******************************************************************************/
enum MOVE_DIRECTION {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

typedef struct snake_s {
    unsigned int head  ;           // Snake head pixel on display
    unsigned int body1 ;           // Snake body1 pixel on display
    unsigned int body2 ;           // Snake body2 pixel on display
    unsigned int tail  ;           // Snake tail pixel on display
    int speed;           // Snake speed
    enum MOVE_DIRECTION direction; // Snake direction
} snake_t;

/******************************************************************************/
/*                                  GLOBAL VARIABLES                          */
/******************************************************************************/
int i = 0;
snake_t snake = {.head=0, .body1=0, .body2=0, .tail=0, .speed=200, .direction=RIGHT};


/******************************************************************************/
/*                                  FUNCTION                                  */
/******************************************************************************/
/**
 * @brief  Configuration of the necessary MCU peripherals
 */
void SystemConfig() {
    /* Turn on all port clocks */
    SIM->SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTE_MASK;

    /* Set corresponding PTA pins (column activators of 74HC154) for GPIO functionality */
    PORTA->PCR[8] = (0 | PORT_PCR_MUX(0x01));   // A0
    PORTA->PCR[10] = (0 | PORT_PCR_MUX(0x01));  // A1
    PORTA->PCR[6] = (0 | PORT_PCR_MUX(0x01));   // A2
    PORTA->PCR[11] = (0 | PORT_PCR_MUX(0x01));  // A3

    /* Set corresponding PTA pins (rows selectors of 74HC154) for GPIO functionality */
    PORTA->PCR[26] = (0 | PORT_PCR_MUX(0x01));  // R0
    PORTA->PCR[24] = (0 | PORT_PCR_MUX(0x01));  // R1
    PORTA->PCR[9] = (0 | PORT_PCR_MUX(0x01));   // R2
    PORTA->PCR[25] = (0 | PORT_PCR_MUX(0x01));  // R3
    PORTA->PCR[28] = (0 | PORT_PCR_MUX(0x01));  // R4
    PORTA->PCR[7] = (0 | PORT_PCR_MUX(0x01));   // R5
    PORTA->PCR[27] = (0 | PORT_PCR_MUX(0x01));  // R6
    PORTA->PCR[29] = (0 | PORT_PCR_MUX(0x01));  // R7

    /* Change corresponding PTA port pins as outputs */
    PTA->PDDR = GPIO_PDDR_PDD(0x3F000FC0);

    /* Set corresponding PTE pins (output enable of 74HC154) for GPIO functionality */
    PORTE->PCR[28] = (0 | PORT_PCR_MUX(0x02));  // #EN

    // Buttons
    PORTE->PCR[10] = PORT_PCR_MUX(0x01); // SW2
    PORTE->PCR[12] = PORT_PCR_MUX(0x01); // SW3
    PORTE->PCR[27] = PORT_PCR_MUX(0x01); // SW4
    PORTE->PCR[26] = PORT_PCR_MUX(0x01); // SW5
    PORTE->PCR[11] = PORT_PCR_MUX(0x01); // SW6

    /* Change corresponding PTE port pins as outputs */
    PTE->PDDR = GPIO_PDDR_PDD(GPIO_PIN(28));

    // Buttons setup - interupts
    NVIC_EnableIRQ(PORTE_IRQn);
    NVIC_SetPriority(PORTE_IRQn, 0);
    PORTE->PCR[10] |= BTN_IRQC_MASK;
    PORTE->PCR[11] |= BTN_IRQC_MASK;
    PORTE->PCR[12] |= BTN_IRQC_MASK;
    PORTE->PCR[26] |= BTN_IRQC_MASK;
    PORTE->PCR[27] |= BTN_IRQC_MASK;

    // Timer setup
    SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
    PIT->MCR &= ~PIT_MCR_MDIS_MASK;
    NVIC_ClearPendingIRQ(PIT0_IRQn);
    NVIC_EnableIRQ(PIT0_IRQn);
    NVIC_SetPriority(PIT0_IRQn, 1);
    PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK; // enable interrupts
    PIT->CHANNEL[0].LDVAL = 0x5fff;
    PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; // start the PIT
}

/**
 * @brief   Function for initialization of the snake structure
 */
void init_snake_body_variables() {
    snake.tail  = (R4 | A0);
    snake.body2 = (R4 | A1);
    snake.body1 = (R4 | A0 | A1);
    snake.head  = (R4 | A2);
    snake.speed = 200;
    snake.direction = RIGHT;
}

/**
 * @brief   Function for waiting
 */
void delay(int t1, int t2) {
    for (int i = 0; i < t1; i++) for (int j = 0; j < t2; j++) ;
}

/**
 * @brief   Function for parse column value from PTA register format
 * @return  Column index
 */
unsigned char get_column_from_display(unsigned int position) {
    unsigned char number = 0x00;
    number |= ((position & A0) >> (A0_PIN - A0_OFSET));
    number |= ((position & A1) >> (A1_PIN - A1_OFSET));
    number |= ((position & A2) >> (A2_PIN - A2_OFSET));
    number |= ((position & A3) >> (A3_PIN - A3_OFSET));
    return number;
}

/**
 * @brief   Function for get column value in PTA register format for set snake pixel column
 * @return  Column index for PTA register
 */
unsigned int get_display_from_column(unsigned char column) {
    unsigned int number = 0x00000000;
    number |= ((column & (1 << A0_OFSET)) << (A0_PIN - A0_OFSET));
    number |= ((column & (1 << A1_OFSET)) << (A1_PIN - A1_OFSET));
    number |= ((column & (1 << A2_OFSET)) << (A2_PIN - A2_OFSET));
    number |= ((column & (1 << A3_OFSET)) << (A3_PIN - A3_OFSET));
    return number;
}

/**
 * @brief   Function for parse row value from PTA register format
 * @return  Row index
 */
unsigned char get_row_from_display(unsigned int position) {
    unsigned char number = 0x00;
    number |= ((position & R0) >> (R0_PIN - R0_OFSET));
    number |= ((position & R1) >> (R1_PIN - R1_OFSET));
    number |= ((position & R2) >> (R2_PIN - R2_OFSET));
    number |= ((position & R3) >> (R3_PIN - R3_OFSET));
    number |= ((position & R4) >> (R4_PIN - R4_OFSET));
    number |= ((position & R5) >> (R5_PIN - R5_OFSET));
    number |= ((position & R6) >> (R6_PIN - R6_OFSET));
    number |= ((position & R7) >> (R7_PIN - R7_OFSET));
    return number;
}

/**
 * @brief   Function for get row value in PTA register format for set snake pixel row
 * @return  Row index for PTA register
 */
unsigned int get_display_from_row(unsigned char row) {
    unsigned int number = 0x00000000;
    number |= ((row & (1 << R0_OFSET)) << (R0_PIN - R0_OFSET));
    number |= ((row & (1 << R1_OFSET)) << (R1_PIN - R1_OFSET));
    number |= ((row & (1 << R2_OFSET)) << (R2_PIN - R2_OFSET));
    number |= ((row & (1 << R3_OFSET)) << (R3_PIN - R3_OFSET));
    number |= ((row & (1 << R4_OFSET)) << (R4_PIN - R4_OFSET));
    number |= ((row & (1 << R5_OFSET)) << (R5_PIN - R5_OFSET));
    number |= ((row & (1 << R6_OFSET)) << (R6_PIN - R6_OFSET));
    number |= ((row & (1 << R7_OFSET)) << (R7_PIN - R7_OFSET));
    return number;
}

/**
 * @brief   Function for set snake pixel
 */
void move_right() {
    unsigned char row = get_row_from_display(snake.head);
    unsigned char column = get_column_from_display(snake.head);
    unsigned int new_row = get_display_from_row(row);
    unsigned int new_column = get_display_from_column(column + 1);
    snake.tail = snake.body2;
    snake.body2 = snake.body1;
    snake.body1 = snake.head;
    snake.head = new_row | new_column;
}

/**
 * @brief   Function for set snake pixel
 */
void move_left() {
    unsigned char row = get_row_from_display(snake.head);
    unsigned char column = get_column_from_display(snake.head);
    unsigned int new_row = get_display_from_row(row);
    unsigned int new_column = get_display_from_column(column - 1);
    snake.tail = snake.body2;
    snake.body2 = snake.body1;
    snake.body1 = snake.head;
    snake.head = new_row | new_column;
}

/**
 * @brief   Function for set snake pixel
 */
void move_up() {
    unsigned char row = get_row_from_display(snake.head);
    unsigned char column = get_column_from_display(snake.head);
    unsigned int new_row = get_display_from_row((row >> 1) ? (row >> 1) : row >> 1 | 0x80);
    unsigned int new_column = get_display_from_column(column);
    snake.tail = snake.body2;
    snake.body2 = snake.body1;
    snake.body1 = snake.head;
    snake.head = new_row | new_column;
}

/**
 * @brief   Function for set snake pixel
 */
void move_down() {
    unsigned char row = get_row_from_display(snake.head);
    unsigned char column = get_column_from_display(snake.head);
    unsigned int new_row = get_display_from_row((row << 1) ? (row << 1) : 0x01);
    unsigned int new_column = get_display_from_column(column);
    snake.tail = snake.body2;
    snake.body2 = snake.body1;
    snake.body1 = snake.head;
    snake.head = new_row | new_column;
}

/**
 * @brief   Function for handle timer interupt for update snake position
 */
void PIT0_IRQHandler() {
    int column_number = i%4;

    // Clear the timer's interrupt flag
    PIT->CHANNEL[0].TFLG = 0x1;

    // Set new snake body position
    if (snake.direction == RIGHT && TIMER_CHANGE_OFSET(i)) {
        move_right();
    } else if (snake.direction == UP && TIMER_CHANGE_OFSET(i)) {
        move_up();
    } else if (snake.direction == DOWN && TIMER_CHANGE_OFSET(i)) {
        move_down();
    } else if (snake.direction == LEFT && TIMER_CHANGE_OFSET(i)) {
        move_left();
    }

    // Place snake onto LED display
    ENABLE_LED_WRITE;
    if      (column_number == 0) { PTA->PDOR = snake.head ; }
    else if (column_number == 1) { PTA->PDOR = snake.body1; }
    else if (column_number == 2) { PTA->PDOR = snake.body2; }
    else if (column_number == 3) { PTA->PDOR = snake.tail ; }
    DISABLE_LED_WRITE;

    //
    i++;
}

/**
 * @brief   Function for handle button interupts to set new snake direction
 */
void PORTE_IRQHandler(void) {
    // Clear interupt flags
    PORTE->ISFR = BTNs_ALL_MASK;

    // BTN_DOWN
    if (IS_CLICK_DOWN && snake.direction == UP   ) {
        snake.direction = RIGHT;
    } else if (IS_CLICK_DOWN && snake.direction == DOWN ) {
        snake.direction = LEFT;
    } else if (IS_CLICK_DOWN && snake.direction == LEFT ) {
        snake.direction = UP;
    } else if (IS_CLICK_DOWN && snake.direction == RIGHT) {
        snake.direction = DOWN;
    }

    // BTN_UP
    if (IS_CLICK_UP && snake.direction == UP   ) {
        snake.direction = LEFT;
    } else if (IS_CLICK_UP && snake.direction == DOWN ) {
        snake.direction = RIGHT;
    } else if (IS_CLICK_UP && snake.direction == LEFT ) {
        snake.direction = DOWN;
    } else if (IS_CLICK_UP && snake.direction == RIGHT) {
        snake.direction = UP;
    }

    // Speed
    if (IS_CLICK_RIGHT && snake.speed > 50) {
            snake.speed -= 50;
    } else if (IS_CLICK_LEFT && snake.speed < 350) {
        snake.speed += 50;
    }

    // Restart game
    if (IS_CLICK_START_STOP) {
        init_snake_body_variables();
    }

    //
    delay(tdelay1, 4);
}

/**
 * @brief   Main function
 */
int main(void) {
    //
    SystemConfig();

    //
    init_snake_body_variables();

    //
    while (1) {;} // Never exit

    //
    return 0;
}


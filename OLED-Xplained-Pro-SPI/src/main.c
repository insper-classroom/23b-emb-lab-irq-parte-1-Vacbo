#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  11
#define BUT_IDX_MASK (1 << BUT_IDX)

// Botão OLED 1
#define BUT1_PIO      PIOD
#define BUT1_PIO_ID   ID_PIOD
#define BUT1_IDX  28
#define BUT1_IDX_MASK (1 << BUT1_IDX)
#define BUT1_WAIT_TIME 350

// Botao OLED 2
#define BUT2_PIO      PIOC
#define BUT2_PIO_ID   ID_PIOC
#define BUT2_IDX  31
#define BUT2_IDX_MASK (1 << BUT2_IDX)

// Botão OLED 3
#define BUT3_PIO      PIOA
#define BUT3_PIO_ID   ID_PIOA
#define BUT3_IDX  19
#define BUT3_IDX_MASK (1 << BUT3_IDX)
#define BUT3_WAIT_TIME 500

volatile char but_flag = 0;
volatile char but1_flag = 0;
volatile char but2_flag = 0;
volatile char but3_flag = 0;
volatile char oled_flag = 0;

void io_init(void);
int pisca_led(int n, int t, int i);
void print_oled(int but_delay);

void but_callback(void)
{
	but_flag = 1;
	oled_flag = 1;
	but2_flag = 0;
}

void but1_callback(void)
{
	if (!pio_get(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK)) {
		but1_flag = 1;
	} else {
		but1_flag = 0;
	}
}

void but2_callback(void) 
{
	but2_flag = 1;
}

void but3_callback(void)
{
	if (!pio_get(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK)) {
		but3_flag = 1;
		} else {
		but3_flag = 0;
	}
}

// pisca led N vez no periodo T
int pisca_led(int n, int t, int i){
  for (;i<n;i++){
    pio_clear(LED_PIO, LED_IDX_MASK);
    delay_ms(t);
    pio_set(LED_PIO, LED_IDX_MASK);
    delay_ms(t);

	gfx_mono_draw_rect(4 + 4*i, 15, 2, 10, GFX_PIXEL_SET);
	
	if (but2_flag) {
		return i+1;
	}
  }
  return i;
}

void print_oled(int but_delay) {
	char freq_str[128];
	sprintf(freq_str, "%d", 500 / but_delay );
	gfx_mono_draw_string("Freq:", 0, 0, &sysfont);
	gfx_mono_draw_string(freq_str, 50, 0, &sysfont);
	gfx_mono_draw_string("Hz", 70, 0, &sysfont);
}

void io_init(void)
{

  // Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);

  // Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);

  // Configura PIO para lidar com o pino do botão como entrada
  // com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	
	pio_set_debounce_filter(BUT_PIO, BUT_IDX_MASK, 60);
	pio_set_debounce_filter(BUT1_PIO, BUT1_IDX_MASK, 60);
	pio_set_debounce_filter(BUT2_PIO, BUT2_IDX_MASK, 60);
	pio_set_debounce_filter(BUT3_PIO, BUT3_IDX_MASK, 60);


  // Configura interrupção no pino referente ao botao e associa
  // função de callback caso uma interrupção for gerada
  // a função de callback é a: but_callback()
  pio_handler_set(BUT_PIO,
                  BUT_PIO_ID,
                  BUT_IDX_MASK,
                  PIO_IT_FALL_EDGE,
                  but_callback);
				  
  pio_handler_set(BUT1_PIO,
				  BUT1_PIO_ID,
	              BUT1_IDX_MASK,
	              PIO_IT_EDGE,
	              but1_callback);
  
  pio_handler_set(BUT2_PIO,
				  BUT2_PIO_ID,
				  BUT2_IDX_MASK,
				  PIO_IT_FALL_EDGE,
				  but2_callback);
  
  pio_handler_set(BUT3_PIO,
				  BUT3_PIO_ID,
				  BUT3_IDX_MASK,
				  PIO_IT_EDGE,
				  but3_callback);

  // Ativa interrupção e limpa primeira IRQ gerada na ativacao
  pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
  pio_enable_interrupt(BUT1_PIO, BUT1_IDX_MASK);
  pio_enable_interrupt(BUT2_PIO, BUT2_IDX_MASK);
  pio_enable_interrupt(BUT3_PIO, BUT3_IDX_MASK);
  
  pio_get_interrupt_status(BUT_PIO);
  pio_get_interrupt_status(BUT1_PIO);
  pio_get_interrupt_status(BUT2_PIO);
  pio_get_interrupt_status(BUT3_PIO);

  
  // Configura NVIC para receber interrupcoes do PIO do botao
  // com prioridade 4 (quanto mais próximo de 0 maior)
  NVIC_EnableIRQ(BUT_PIO_ID);
  NVIC_EnableIRQ(BUT1_PIO_ID);
  NVIC_EnableIRQ(BUT2_PIO_ID);
  NVIC_EnableIRQ(BUT3_PIO_ID);
  
  NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
  NVIC_SetPriority(BUT1_PIO_ID, 3);
  NVIC_SetPriority(BUT2_PIO_ID, 1);
  NVIC_SetPriority(BUT3_PIO_ID, 3);
}

int main (void)
{
	board_init();
	sysclk_init();
	delay_init();

	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

	// configura botao com interrupcao
	io_init();
	
	// Init OLED
	gfx_mono_ssd1306_init();
	
	// Declare local variables
	int but_delay = 100;
	int i = 0;
	
  /* Insert application code here, after the board has been initialized. */
	while(1) {
		if (oled_flag) {
			print_oled(but_delay);
			oled_flag = 0;
		}
		if (but2_flag) {
			while (but2_flag) {}
		}
		if (but_flag) {
			i = pisca_led(30, but_delay, i);
			if (i >= 30) {
				i = 0;
				gfx_mono_draw_filled_rect(0, 15, 128, 10, GFX_PIXEL_CLR);
			}
			but_flag = 0;
		}
		if (but1_flag) {
			delay_ms(BUT1_WAIT_TIME);
			if (but1_flag) {
				but1_flag = 0;
				but_delay += 100;
			} else if (but_delay > 100) {
				but_delay -= 100;
			}
			oled_flag = 1;	
		}
		if (but3_flag) {
			delay_ms(BUT3_WAIT_TIME);
			if (!but3_flag) {
				but_delay += 100;
			}
			oled_flag = 1;
		}

		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);			
	}
}

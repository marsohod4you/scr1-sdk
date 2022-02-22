#include <stdio.h>
#include PLATFORM_HDR
#include <leds.h>
#include <rtc.h>

//turn ON/OFF board LEDs according to "val" bits
void set_leds( unsigned int val )
{
#ifdef PLF_PINLED_NUM
	sc1f_leds_set( val );
#endif // PLF_PINLED_NUM
}

//delay for "ms" milliseconds
void delay( long ms )
{
	sys_tick_t time0;
	sys_tick_t time1;
	sys_tick_t delay_in_ticks;

	delay_in_ticks = ms2ticks(ms);
	
	time0 = now();
	while(1)
	{
		time1 = now();
		if( (time1 - time0) > delay_in_ticks )
			return;
    }
}

#ifdef PLF_HEXLED_ADDR

#define MAX_HEX_LEDS 8

static const uint8_t HEX_DIGITS_TABLE[16] = {
    HEXLED_0,
    HEXLED_1,
    HEXLED_2,
    HEXLED_3,
    HEXLED_4,
    HEXLED_5,
    HEXLED_6,
    HEXLED_7,
    HEXLED_8,
    HEXLED_9,
    HEXLED_A,
    HEXLED_B,
    HEXLED_C,
    HEXLED_D,
    HEXLED_E,
    HEXLED_F
};

struct hex_seg_map {
    unsigned long addr;
    unsigned shift;
};
#endif // PLF_HEXLED_ADDR

#ifdef PLF_HEXLED_ADDR_MAP
static const struct hex_seg_map HEX_MAP[] = {
    PLF_HEXLED_ADDR_MAP
};
#endif // PLF_HEXLED_ADDR_MAP

#if PLF_HEXLED_PORT_WIDTH == 4
typedef uint32_t pinled_port_mem;
#elif PLF_HEXLED_PORT_WIDTH == 2
typedef uint16_t pinled_port_mem;
#else
typedef uint8_t pinled_port_mem;
#endif

void sc1f_leds_hex(unsigned n, unsigned v)
{
#ifdef PLF_HEXLED_ADDR_MAP
    if (n < sizeof(HEX_MAP) / sizeof(*HEX_MAP)) {
        volatile pinled_port_mem *p = (volatile pinled_port_mem*)(HEX_MAP[n].addr);
        unsigned shift = HEX_MAP[n].shift;
		pinled_port_mem mask = ~(0xff << shift);
        *p = (*p & mask) | ((v ^ PLF_HEXLED_INV) << shift);
    }
#elif defined(PLF_HEXLED_ADDR) // PLF_HEXLED_ADDR_MAP
    volatile pinled_port_mem *p = (volatile pinled_port_mem*)PLF_HEXLED_ADDR;
    p[n] = v ^ PLF_HEXLED_INV;
#endif // PLF_HEXLED_ADDR_MAP
}

void sc1f_leds_hex_digits( unsigned val )
{
	for( int i=0; i<4; i++ )
	{
		unsigned v = val>>(i*4);
		sc1f_leds_hex(3 - i + 2, HEX_DIGITS_TABLE[v & 0xf]);
	}
}

unsigned int get_dip_sw()
{
	volatile unsigned int* port = PLF_DIP_ADDR;
	return *port;
}

int main(void)
{
    printf("Hello, SCR!\n");
	unsigned int key = get_dip_sw();
	printf("KEY: %08X\n",key);
	
	unsigned int K=0;
	int A = 1;
	while(1)
	{
		unsigned int key2 = get_dip_sw();
		if( key!=key2 )
		{
			key = key2;
			if( key&1 ) A=1;
			else if(key&2) A=-1;
			if( key&3 )
				printf("KEY: %08X count %s\n",key, (A==1)?"UP":"DOWN");
			else
				printf("KEY: %08X\n",key);
		}
			
		delay(100);
		sc1f_leds_hex_digits(K);
		set_leds(K);
		K=K+A;
	}
    return 0;
}

#include <stdio.h>
#include <sleep.h>
#include <stdbool.h>
#include "main.h"
#include "blocks.h"

#define MAX_LED 		0x8000
#define BASE_GPIO0 		0x40000000
#define BASE_GPIO1 		0x40010000
#define MASK_BUTTONS	0xF

color_t amar = {255,255,0};
position_t pos_barra={80,100};//75
bola_t bola;
block_t map[N_BLOCKS_X][N_BLOCKS_Y];
position_t frames[4];
int bola_activa=0;

volatile int *gpio0 = (int*)BASE_GPIO0; // dir base buttons

color_t pixel(int x, int y){
	color_t col;
	int *ptr = (int*) VGA_CTRL_BASE;
	int val = ptr[(y<<8) | x];
	col.r = (val&0xf)<<4;
	col.b = (val&0xf0);
	col.g = (val&0xf00)>>4;
	return col;
}

int main(){
	int i,j, count1=0, speed_ball=0;
	init_botones();
	int dir;
	game_status_t status = continues;

	// Pinta el fondo negro. Se puede reemplazar por una llamada a rect
    for(i=0;i<160;i++)
    	for(j=0;j<120;j++)
    		pinta(i,j,negro);

    // Pinta bordes
    frames[0].x = INT_X_BORDER; frames[0].y = INT_Y_BORDER;
    frames[1].x = END_X_BORDER; frames[1].y = INT_Y_BORDER;
    frames[2].x = INT_X_BORDER; frames[2].y = END_Y_BORDER;
    frames[3].x = INT_X_BORDER; frames[3].y = INT_Y_BORDER;

    rect(frames[0], blanco, END_X_BORDER, BORDER_THICKNESS);
    rect(frames[1], blanco, BORDER_THICKNESS, END_Y_BORDER);
    rect(frames[2], blanco, END_X_BORDER, BORDER_THICKNESS);
    rect(frames[3], blanco, BORDER_THICKNESS, END_Y_BORDER);

    // Print map
    init_map(map);
    print_map(map);

    // Pos inicial de la nave
	init_bola();
	mueve_barra(0);
	bola.mov = 3;

    while(1){
    	// una de cada 5 veces, mueve la nave en el eje X
    	if(count1==3){
    		count1=0;
    		if ((dir = espera_boton()) != 0)
				mueve_barra(dir);
    	}else count1++;
    	// Mueve la bala cada vuelta del ciclo. La bala va 5 veces m�s r�pido que la nave.
    	if(speed_ball == 5){
    		status = mueve_bola();
    		speed_ball=0;
    	} else speed_ball++;
    	usleep(10000);

    	if (status == game_over)
    	{
    		init_bola();
    		count1 = 0;
			speed_ball = 0;
			status = continues;
    	}
    }

	return 0;
}

void init_botones(){
	gpio0[1] = MASK_BUTTONS; 						// Configurar 4 bits como entrada
}

int espera_boton(){
	int data;
	char btn = 0;

	if ((data = gpio0[0] & MASK_BUTTONS) != 0) { 	// Espera a que se pulse un boton.
		if ((data & 0x2) != 0)			// Boton de mover a la izquierda
			btn = 1;
		else if ((data & 0x4) != 0)			// Boton de mover a la derecha
			btn = 2;
		xil_printf("Boton pulsado %d\r", btn);

	}
	return btn;
}

int equals_color(color_t color1, color_t color2)
{
	return ((color1.r == color2.r) && (color1.g == color2.g) && (color1.b == color2.b));
}

game_status_t mueve_bola()
{
	game_status_t status = continues;
	if(bola_activa)
	{
		position_t next_pos;
		side_t side;
		block_t block;

		movement_t next_mov;
		bool is_block = false;
		color_t next_color;
		switch(bola.mov)
		{
			case 0:
				next_pos.x = bola.x+1;
				next_pos.y = bola.y-1;
				break;
			case 1:
				next_pos.x = bola.x+1;
				next_pos.y = bola.y+1;
				break;
			case 2:
				next_pos.x = bola.x-1;
				next_pos.y = bola.y+1;
				break;
			case 3:
				next_pos.x = bola.x-1;
				next_pos.y = bola.y-1;
				break;
		}
		next_color = pixel(next_pos.x, next_pos.y);
		if (!equals_color(negro, next_color))
		{
			if ((side = calculate_border(next_pos)) == not_border)
			{
				if ((is_block = calculate_block(next_pos, &block)))
				{
					block.collisions--;
					print_block(block.location, negro);
				}
			}
			else if (side == bottom)
			{
				pinta(bola.x, bola.y, negro);
				return game_over;
			}
			else
				is_block = false;

			next_mov = calculate_rebound(bola, side, is_block, &block, &next_pos);	// lo ha hecho Pablo el cutres
		} else
		{
			next_mov = bola.mov;
		}
		pinta(bola.x, bola.y, negro);
		bola.x = next_pos.x;
		bola.y = next_pos.y;
		bola.mov = next_mov;
		pinta(bola.x, bola.y, azul_claro);
	}
	return status;
}

side_t calculate_border(position_t next_pos)
{
	bool is_left = false, is_right = false, is_top = false, is_bottom = false;
	side_t res;

	if (next_pos.x == INT_X_BORDER)
		is_left = true;
	if (next_pos.x == END_X_BORDER)
		is_right = true;
	if (next_pos.y == INT_Y_BORDER)
		is_top = true;
	if (next_pos.y == END_Y_BORDER)
		is_bottom = true;

	if (is_left)
		if (is_top)
			res = top_left;
		else if (is_bottom)
			res = bottom_left;
		else
			res = left;
	else if (is_right)
		if (is_top)
			res = top_right;
		else if (is_bottom)
			res = bottom_right;
		else
			res = right;
	else if (is_top)
		res = top;
	else if (is_bottom)
		res = bottom;
	else
		res = not_border;


	return res;
}

side_t which_side_block(position_t next_pos, block_t block)
{
	bool is_left = false, is_right = false, is_top = false, is_bottom = false;
	side_t res;

	if (next_pos.x >= block.location.x && next_pos.x < block.location.x + BLOCK_LENGTH &&
			next_pos.y == block.location.y)
		is_top = true;
	if (next_pos.x >= block.location.x && next_pos.x < block.location.x + BLOCK_LENGTH &&
			next_pos.y == block.location.y + BLOCK_HEIGHT - 1)
		is_bottom = true;
	if (next_pos.x == block.location.x && next_pos.y >= block.location.y &&
			next_pos.y < block.location.y + BLOCK_HEIGHT)
		is_left = true;
	if (next_pos.x == block.location.x + BLOCK_LENGTH - 1 && next_pos.y >= block.location.y &&
			next_pos.y < block.location.y + BLOCK_HEIGHT)
		is_right = true;

	if (is_left)
		if (is_top)
			res = top_left;
		else if (is_bottom)
			res = bottom_left;
		else
			res = left;
	else if (is_right)
		if (is_top)
			res = top_right;
		else if (is_bottom)
			res = bottom_right;
		else
			res = right;
	else if (is_top)
		res = top;
	else if (is_bottom)
		res = bottom;

	return res;
}

side_t which_side_bar(position_t next_pos)
{
	bool is_left = false, is_right = false, is_top = false;
	side_t res;

	if (next_pos.x >= pos_barra.x && next_pos.x < pos_barra.x + BAR_LENGTH &&
			next_pos.y == pos_barra.y)
		is_top = true;
	if (next_pos.x == pos_barra.x && next_pos.y >= pos_barra.y &&
			next_pos.y < pos_barra.y + BAR_HEIGHT)
		is_left = true;
	if (next_pos.x == pos_barra.x + BAR_LENGTH - 1 && next_pos.y >= pos_barra.y &&
			next_pos.y < pos_barra.y + BAR_HEIGHT)
		is_right = true;

	if (is_left)
		if (is_top)
			res = top_left;
		else
			res = left;
	else if (is_right)
		if (is_top)
			res = top_right;
		else
			res = right;
	else if (is_top)
		res = top;

	return res;
}


bool calculate_block(position_t next_pos, block_t *block)
{
	block_t res;
	for (int i = N_BLOCKS_Y - 1; i >= 0; i--)
	{
		for (int j = N_BLOCKS_X - 1; j >= 0; j--)
		{
			res = map[j][i];
			if (next_pos.x >= res.location.x && next_pos.x < res.location.x + BLOCK_LENGTH &&
				next_pos.y >= res.location.y && next_pos.y < res.location.y + BLOCK_HEIGHT)
			{
				*block = res;
				return true;
			}

		}
	}
	return false;
}

movement_t calculate_rebound(bola_t bola, side_t side, bool is_block, block_t *block, position_t *next_pos)
{
	movement_t mov = bola.mov;
	if (side != not_border) 	// border
	{
		switch (side) {
			case top_left:
				mov = mov_bottom_right;
				next_pos->x = bola.x + 1;
				next_pos->y = bola.y + 1;
				break;
			case top_right:
				mov = mov_bottom_left;
				next_pos->x = bola.x - 1;
				next_pos->y = bola.y + 1;
				break;
			case top:
				if (bola.mov == mov_top_right) {
					mov = mov_bottom_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y + 1;
				}
				else {
					mov = mov_bottom_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y + 1;
				}
				break;
			case left:
				if (bola.mov == mov_top_left) {
					mov = mov_top_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_bottom_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y + 1;
				}
				break;
			case right:
				if (bola.mov == mov_top_right) {
					mov = mov_top_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_bottom_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y + 1;
				}
				break;
			default:
				break;
		}
	} else if (is_block) 	// block
	{
		switch (which_side_block(*next_pos, *block)) {
			case top_left:
				mov = mov_top_left;
				next_pos->x = bola.x - 1;
				next_pos->y = bola.y - 1;
				break;
			case top_right:
				mov = mov_top_right;
				next_pos->x = bola.x + 1;
				next_pos->y = bola.y - 1;
				break;
			case bottom_left:
				mov = mov_bottom_left;
				next_pos->x = bola.x - 1;
				next_pos->y = bola.y + 1;
				break;
			case bottom_right:
				mov = mov_bottom_right;
				next_pos->x = bola.x + 1;
				next_pos->y = bola.y + 1;
				break;
			case top:
				if (bola.mov == mov_bottom_right) {
					mov = mov_top_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_top_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y - 1;
				}
				break;
			case bottom:
				if (bola.mov == mov_top_right) {
					mov = mov_bottom_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y + 1;
				}
				else {
					mov = mov_bottom_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y + 1;
				}
				break;
			case left:
				if (bola.mov == mov_bottom_right) {
					mov = mov_bottom_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y + 1;
				}
				else {
					mov = mov_top_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y - 1;
				}
				break;
			case right:
				if (bola.mov == mov_bottom_left) {
					mov = mov_top_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_bottom_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y + 1;
				}
				break;
			default:
				break;
		}
	} else if (next_pos->y < END_Y_BORDER)
	{
		switch (which_side_bar(*next_pos)) {
			case top_left:
				mov = mov_top_left;
				next_pos->x = bola.x - 1;
				next_pos->y = bola.y - 1;
				break;
			case top_right:
				mov = mov_top_right;
				next_pos->x = bola.x + 1;
				next_pos->y = bola.y - 1;
				break;
			case top:
				if (bola.mov == mov_bottom_right) {
					mov = mov_top_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_top_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y - 1;
				}
				break;
			case left:
				if (bola.mov == mov_bottom_right) {
					mov = mov_bottom_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y + 1;
				}
				else {
					mov = mov_top_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y - 1;
				}
				break;
			case right:
				if (bola.mov == mov_bottom_left) {
					mov = mov_top_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_bottom_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y + 1;
				}
				break;
			default:
				break;
		}
	}
	return mov;
}



void pinta(int x, int y, color_t rgb){
	int *ptr = (int *)VGA_CTRL_BASE;
	int val = (rgb.r>>4) | (rgb.b&0xf0) | ((rgb.g&0xf0)<<4);
	ptr[(y<<8)| x]=val;
}

void rect (position_t pos, color_t col, int w, int h){
	int i,j;
	for(i=0;i<w;i++){
		for(j=0;j<h;j++){
			pinta(pos.x+i,pos.y+j,col);
		}
	}
}

void imagen_11x11(position_t pos, color_t img[11][2]){
	int i,j;
	struct color alfa = negro;
	for(i=0;i<BAR_LENGTH;i++){
		for(j=0;j<2;j++){
			if(img[j][i].r==alfa.r && img[j][i].g==alfa.g && img[j][i].b==alfa.b) continue;
			else pinta(pos.x+i, pos.y+j, img[j][i]);
		}
	}
}

void mueve_barra(int dir){
	static int var = 0;
	if (dir == 1)
	{
		var = -1;
	}
	else if (dir == 2)
	{
		var = 1;
	}

	if (pos_barra.x + var >= 2 && pos_barra.x + var <= 147) {
		rect(pos_barra,negro,BAR_LENGTH,BAR_HEIGHT);
		pos_barra.x = pos_barra.x + var;
		rect(pos_barra,gold,BAR_LENGTH,BAR_HEIGHT);
	}
}

void init_bola(){
	bola.x=pos_barra.x + BAR_LENGTH / 2;
	bola.y=pos_barra.y-1;
	bola_activa=1;
	pinta(bola.x,bola.y,azul_claro);
}

/************************************
 * This is the framework for those
 * who wish to write their own C
 * code for the basic badge
 ************************************/


#include "badge_user.h"

void user_program_init(void)
	{
	/* This will run when User Program is first selected form the menu */
	clr_buffer();
	video_gotoxy(5,4);
    stdio_write("Use this example to write");
	video_gotoxy(5,5);
	stdio_write("your own C code for the badge.");
	}
enum Action{
    MOVE_LEFT,
    MOVE_RIGHT,
    FIRE,
    RESET,
    NONE
};

enum Action read_action() {
    int8_t sstr[3];
    uint8_t get_stat = stdio_get(sstr);
    if (get_stat!=0)
    {
        if (sstr[0]==K_ENT) {	return FIRE;	}
        if (sstr[0]==K_LT) {	return MOVE_LEFT;	}
        if (sstr[0]==K_RT) {	return MOVE_RIGHT;	}
        if (sstr[0]==K_DN) {	return RESET;	}
    }
    return NONE;
}

char* action_to_string(enum Action action) {
    switch(action) {
        case MOVE_LEFT: return "MOVE_LEFT"; 
        case MOVE_RIGHT: return "MOVE_RIGHT"; 
        case FIRE: return "FIRE"; 
        case NONE: return "NONE";
        case RESET: return "RESET";
        default: return "NISPE";
    }
    return "";
}
#define BULLET_WIDTH 8
#define BULLET_HEIGHT 8
#define PLAYER_STEP_SIZE 16
#define PLAYER_WIDTH 3 * BULLET_WIDTH
#define PLAYER_HEIGHT 8
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define BULLET_STEP_SIZE (BULLET_HEIGHT * 2)

void clear_screen() {
    tft_fill_area (0, 0, 320, 240, 0);
}

void draw_player(int x) {
    tft_fill_area(x, SCREEN_HEIGHT - PLAYER_HEIGHT, PLAYER_WIDTH, PLAYER_HEIGHT, 0x00FFFFFF);
    tft_fill_area(x + (PLAYER_WIDTH / 3), SCREEN_HEIGHT - (PLAYER_HEIGHT * 2), PLAYER_WIDTH / 3, PLAYER_HEIGHT, 0x00FAAB18);
}

struct BulletForMyValentine {
    int x;
    int y;
    int power_level;//over 9000
    int alive;
};
int check_collision(struct BulletForMyValentine* bullet) {
    if(bullet->y <= 0) {
        bullet->alive = 0;
        return 1;
    }
    
    return 0;
}
int check_enemy_collision(struct BulletForMyValentine* bullet) {
    if(bullet->y > SCREEN_HEIGHT - PLAYER_HEIGHT) {
        bullet->alive = 0;
        return 1;
    }
    return 0;
}
int enemy_hit_me(struct BulletForMyValentine* bullet, int player_x){
    if(bullet->y > SCREEN_HEIGHT - PLAYER_HEIGHT) {
        if(bullet->x >= player_x && bullet->x <= (player_x + PLAYER_WIDTH) ){
            return 1;
        }
    }
    return 0;
}

void draw_bullet(struct BulletForMyValentine* bullet) {
    tft_fill_area(bullet->x, bullet->y - BULLET_HEIGHT, BULLET_WIDTH, BULLET_HEIGHT, 0x00FF0000);
}
void draw_enemy_bullet(struct BulletForMyValentine* bullet) {
    tft_fill_area(bullet->x, bullet->y - BULLET_HEIGHT, BULLET_WIDTH, BULLET_HEIGHT, 0x000000FF);
}
void game_end(int win)
{
    enable_display_scanning(1);
    memset(disp_buffer, 0, sizeof(disp_buffer));
    video_gotoxy(10, 10);
    if(win)
    {
        stdio_write("Congratulations, you win");
    }
    else
    {
        stdio_write("You loose, bummer :(");
    }
    wait_ms(2000);
}
#define BULLETS_NUM 5
enum GameState { INIT, START, RUNNING, WIN, LOOSE } state = INIT;
void user_program_loop(void)
{
    int player_x = 0;
    long blum = 0;
    struct BulletForMyValentine bullets[BULLETS_NUM];
    struct BulletForMyValentine enemy_bullets[BULLETS_NUM];
        
    if(state == INIT) {
        enable_display_scanning(1);
        memset(disp_buffer, 0, sizeof(disp_buffer));
        video_gotoxy(10, 10);
        stdio_write("Press any key to start");
        int i_ready = 0;
        int ti_ready = 0;
        while(1) {
            int8_t sstr[3];
            uint8_t get_stat = stdio_get(sstr);
            if(get_stat) {
                tx_write(0b11000000);
                i_ready = 1;
                video_gotoxy(10, 10);
                stdio_write("Waiting for other player");
            }
            if(rx_sta() != 0) {
                if(rx_read() == 0b11000000){
                    ti_ready = 1;
                }
            }
            if(i_ready && ti_ready) {
                state = START;
                break;
            }
        }
    } else if(state == START) {
        clear_screen();
        enable_display_scanning(0);
        player_x = 320/2;
        draw_player(player_x);
        blum = millis();
        memset(bullets, 0, sizeof(struct BulletForMyValentine) * BULLETS_NUM);
        memset(enemy_bullets, 0, sizeof(struct BulletForMyValentine) * BULLETS_NUM);
        state = RUNNING;
    } else if(state == RUNNING) {
        player_x = 320/2;
        blum = millis();
        while(1) //Loop forever
        {
            int action_occured = 0;
            char serial_recv;   
            if(rx_sta()!=0) {
                serial_recv = rx_read();
                int command = (serial_recv & 0b11000000) >> 6;
                int enemy_x = serial_recv & 0b00111111;
                if(command == 1) 
                {  
                    int i = 0;
                    for(;i < BULLETS_NUM; i++) {
                        if(!enemy_bullets[i].alive) {
                            enemy_bullets[i].alive = 1;
                            enemy_bullets[i].x = 320 - ((enemy_x * BULLET_WIDTH) + BULLET_WIDTH);
                            enemy_bullets[i].y = 0;          
                            break;
                        }
                    }   
                }
                else if(command == 2){
                    state = WIN;
                    break;
                }
            }
            
            enum Action action = read_action();    
            if(action == MOVE_RIGHT) {
                if(player_x  < SCREEN_WIDTH - (PLAYER_WIDTH + PLAYER_STEP_SIZE)) {
                    player_x += PLAYER_STEP_SIZE;
                    action_occured = 1;
                }
            }
            if(action == MOVE_LEFT) {
                if(player_x  > 0) {
                    player_x -= PLAYER_STEP_SIZE;
                    action_occured = 1;
                }
            }
            if(action == FIRE) {
                int i = 0;
                for(;i < BULLETS_NUM; i++) {
                    if(!bullets[i].alive) {
                        bullets[i].alive = 1;
                        bullets[i].x = player_x + BULLET_WIDTH;
                        bullets[i].y = SCREEN_HEIGHT - BULLET_HEIGHT;
                        break;
                    }
                }
            }
            if((millis() - blum) > 40) {
                blum = millis();
                int i = 0;
                for(; i < BULLETS_NUM; i++) {
                    if(bullets[i].alive) {
                        bullets[i].y -= BULLET_STEP_SIZE;
                        if(check_collision(&bullets[i])) {
                            int command = 0b01000000 | (bullets[i].x / BULLET_WIDTH);
                            tx_write(command);
                        }
                        action_occured = 1;
                    }
                    if(enemy_bullets[i].alive) {
                        enemy_bullets[i].y += BULLET_STEP_SIZE;
                        if(check_enemy_collision(&enemy_bullets[i])) {
                        }
                        if(enemy_hit_me(&enemy_bullets[i], player_x)){
                            //i have been hit
                            int command = 0b10000000;
                            tx_write(command);
                            state = LOOSE;     
                            break;
                        }
                            
                        action_occured = 1;
                    }
                }
            }
            if(action_occured) {
                clear_screen();
                draw_player(player_x);
                int i = 0;
                for(;i<BULLETS_NUM;i++) {
                    if(bullets[i].alive) {
                        draw_bullet(&bullets[i]);
                    }
                    if(enemy_bullets[i].alive) {
                        draw_enemy_bullet(&enemy_bullets[i]);
                    }
                }
            }
            if(state != RUNNING) {
                break;
            }
        }
    } else if(state == WIN) {
        game_end(1);
        while(1) {
            int8_t sstr[3];
            uint8_t get_stat = stdio_get(sstr);
            if(get_stat) {
                state = INIT;
                break;
            }
        }
    }else if(state == LOOSE) {
        game_end(0);
        while(1) {
            int8_t sstr[3];
            uint8_t get_stat = stdio_get(sstr);
            if(get_stat) {
                state = INIT;
                break;
            }
        }
    }
        
}


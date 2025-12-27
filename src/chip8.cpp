#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t memory[0x1000] = {};
uint8_t registers[16] = {};
uint16_t addr_reg = 0;
uint16_t stack[32] = {};
uint16_t sp = 0, pc = 0x200;
uint8_t screen[64*32] = {};
uint8_t halt = 0; // reason 1: none; stay halted  2: waiting for keypress
uint8_t delay = 0;
uint8_t sound = 0;
uint8_t key = 0xFF; // 0xFF: invalid no key
uint8_t keyboard[16] = {};

#define SCREENSCALE 20

// Font provided by http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter
uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, //0
    0x20, 0x60, 0x20, 0x20, 0x70, //1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
    0x90, 0x90, 0xF0, 0x10, 0x10, //4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
    0xF0, 0x10, 0x20, 0x40, 0x40, //7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
    0xF0, 0x90, 0xF0, 0x90, 0x90, //A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
    0xF0, 0x80, 0x80, 0x80, 0xF0, //C
    0xE0, 0x90, 0x90, 0x90, 0xE0, //D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
    0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

// Key mapping
// Original     Emulator
// 1 2 3 C      1 2 3 4
// 4 5 6 D      Q W E R
// 7 8 9 E      A S D F
// A 0 B F      Z X C V
int sdl_key_map[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v
    };


void run_cycle(bool verbose)
{
    uint16_t opcode = ((uint16_t)memory[pc] << 8) + memory[pc+1];
    if(verbose)
      printf("Exec: %04x: %04x\n", pc, opcode);
    pc+=2;
    switch(opcode >> 12) {
        case 0:
            if((opcode & 0x0FF) == 0xEE) {
                pc = stack[--sp];
            }
            else if((opcode & 0x0FF) == 0xE0) {
                memset(screen, 0, sizeof(screen));
            }
            else goto err;
            break;
        case 1:
            pc = opcode & 0x0FFF;
            break;
        case 2:
            stack[sp++] = pc;
            pc = opcode & 0x0FFF;
            break;
        case 3:
            if(registers[(opcode & 0x0F00)>>8] == (opcode&0x00FF))
                pc+=2;
            break;
        case 4:
            if(registers[(opcode & 0x0F00)>>8] != (opcode&0x00FF))
                pc+=2;
            break;
        case 5:
            if(registers[(opcode & 0x0F00)>>8] == registers[(opcode & 0x00F0)>>4])
                pc+=2;
            break;
        case 6:
            registers[(opcode & 0x0F00) >> 8] = (opcode & 0x0FF);
            break;
        case 7:
            registers[(opcode & 0x0F00) >> 8] += (opcode & 0x0FF);
            break;
        case 8:
            {
                uint8_t *r1 = registers + ((opcode & 0x0F00) >> 8);
                uint8_t *r2 = registers + ((opcode & 0x00F0) >> 4);
                uint8_t *f = registers + 0xF;
                uint8_t op = (opcode & 0xF);
                uint8_t flag = 0;
                if(op == 0)
                    *r1 = *r2;
                else if (op == 1)
                    *r1 |= *r2, *f=0;
                else if (op == 2)
                    *r1 &= *r2, *f=0;
                else if (op == 3)
                    *r1 ^= *r2, *f=0;
                else if (op == 4)
                    flag = (*r1 + *r2 > 255 ? 1 : 0), *r1 += *r2, *f = flag;
                else if (op == 5)
                    flag = (*r1 - *r2 < 0 ? 0 : 1), *r1 -= *r2, *f = flag;
                else if (op == 6)
                    *r1=*r2, flag = (*r1 & 1), *r1 >>= 1, *f = flag;
                else if (op == 7)
                    flag = (*r2 - *r1 < 0 ? 0 : 1), *r1 = *r2 - *r1, *f = flag;
                else if (op == 0xE)
                    *r1=*r2, flag = ((*r1 & 0x80) ? 1 : 0), *r1 <<= 1, *f = flag;
                else goto err;
            }
            break;
        case 9:
            if(registers[(opcode & 0x0F00)>>8] != registers[(opcode & 0x00F0)>>4])
                pc+=2;
            break;
        case 0xA:
            addr_reg = opcode & 0x0FFF;
            break;
        case 0xB:
            pc = registers[0] + (opcode & 0x0FFF);
            break;
        case 0xC:
        {
            uint8_t r = rand() & 0xFF & (opcode & 0x00FF);
            registers[(opcode & 0x0F00) >> 8] = r;
        }
            break;
        case 0xD:
            {
                uint8_t px = registers[(opcode & 0x0F00) >> 8] & 0x3F;
                uint8_t py = registers[(opcode & 0x00F0) >> 4] & 0x1F;
                uint8_t n = (opcode & 0x000F);
                registers[15]=0;
                for(int y = 0; y < n; ++y)
                {
                    uint8_t row = memory[addr_reg+y];
                    for(int x = 7; x >= 0; --x)
                    {
                        if(px+x < 0x40 && py+y < 0x20)
                        {
                            uint8_t *s = screen + (((px + x)&0x3F) + ((py+y)&0x1F)*64);
                            if(*s && (row&1)) { *s = 0; registers[15]=1; }
                            else if (row&1) *s = 1;
                        }
                        row >>= 1;
                    }
                }
            }
            break;
        case 0xE:
            if ((opcode & 0x00FF) == 0x9E) {
                if(keyboard[registers[(opcode & 0x0F00) >> 8] & 0xF])
                    pc += 2;
            }
            else if ((opcode & 0x00FF) == 0xA1) {
                if(!keyboard[registers[(opcode & 0x0F00) >> 8] & 0xF])
                    pc += 2;
            }
            else goto err;
            break;
        case 0xF:
            {
                uint8_t *r1 = registers + ((opcode & 0x0F00) >> 8);
                uint8_t op = opcode & 0x00FF;
                if(op == 0x07)
                    *r1 = delay;
                else if (op == 0x0A)
                    if(key == 0xFF) {
                        // no key, halt until next press
                        halt = 2;
                        pc -= 2;
                    }
                    else {
                        *r1 = key;
                        key = 0xFF;
                    }
                else if (op == 0x15)
                    delay = *r1;
                else if (op == 0x18)
                    sound = *r1;
                else if (op == 0x1E)
                    addr_reg += *r1;
                else if (op == 0x29)
                    addr_reg = *r1 * 5;
                else if (op == 0x33) {
                    uint8_t v = *r1; memory[addr_reg] = v/100; v -= v/100*100;
                    memory[addr_reg+1] = v/10;
                    v -= v/10*10;
                    memory[addr_reg+2] = v;
                }
                else if (op == 0x55)
                {
                    memcpy(memory+addr_reg, registers, r1-registers+1);
                    addr_reg += r1-registers+1;
                }
                else if (op == 0x65)
                {
                    memcpy(registers, memory+addr_reg, r1-registers+1);
                    addr_reg += r1-registers+1;
                }
                else
                    goto err;
            }
            break;
        default:
err:
            printf("pc: %04x\n", pc);
            printf("Unknown opcode %04x\n", opcode);
            printf("Execution halted\n");
            halt = 1;
            break;
    }
}


int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "Chip8 emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        64*SCREENSCALE, 32*SCREENSCALE,
        SDL_WINDOW_SHOWN | SDL_WINDOW_INPUT_FOCUS
    );

    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_ShowWindow(window);
    SDL_RaiseWindow(window);

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("SDL2 initialized successfully!\n");
    printf("Window: 640x480\n");
    printf("Press ESC to quit\n");

    // Builtin fontset stored in from from addr 0
    memcpy(memory, fontset, 80);

    // Rom loading
    if(argc > 1) {
	    printf("Loading rom %s\n", argv[1]);
	    FILE* f = fopen(argv[1], "rb");
	    fseek(f, 0, SEEK_END);
	    uint64_t romsize = ftell(f);
	    fseek(f, 0, SEEK_SET);
	    if(romsize > 0x1000 - 0x200) {
            printf("Rom too big (%lld bytes)\n", romsize);
            return 1;
        }
        fread(memory + 0x200, romsize, 1, f);
        fclose(f);	    
    }

    bool running = true;
    bool stepping = false;
    int framecount = 0;
    int icount = 0;

    while (running) {
        framecount++;

        if(false && framecount%60 == 0)
        {
            printf("Frames %i, instructions %i, time: %f\n", framecount, icount, float(SDL_GetTicks())/(CLOCKS_PER_SEC/1000));
        }

        int cycles = 8;

        if(stepping)
            cycles = 0;

	    // Keyboard handling
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
            {
                if(stepping)
                    cycles = 1;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                } else {
                    for(int i = 0; i < 16; ++i)
                    {
                        if(event.key.keysym.sym == sdl_key_map[i])
                        {
                            keyboard[i] = event.type == SDL_KEYDOWN ? 1 : 0;
                            if(halt == 2 && event.type == SDL_KEYUP)
                            {
                                key = i;
                                halt = 0;
                            }
                            break;
                        }
                    }
                }
            }
        }

        // Run cpu
        while (!halt && cycles--)
        {
            icount++;
            run_cycle(stepping);
        }

        // Update timers
        if(delay > 0)
            delay--;

        if(sound > 0)
            sound--;

        // Clear screen
    	SDL_SetRenderDrawColor(renderer, sound > 0 ? 80 : 5, 15, 5, 255);
        SDL_RenderClear(renderer);

        // Draw screen
        SDL_SetRenderDrawColor(renderer, 20, 128, 20, 255);
        for(int x = 0; x < 64; ++x)
        {
            for(int y = 0; y < 32; ++y)
            {
                if(screen[x+y*64])
                {
                    SDL_Rect rect = {x*SCREENSCALE, y*SCREENSCALE, SCREENSCALE, SCREENSCALE};
                    SDL_RenderFillRect(renderer, &rect);
                }
            }
        }

	    // Flip
        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    printf("Shutting down...\n");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

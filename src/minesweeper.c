#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
 
#include <SDL.h>
 
/* ********************************* */
/* *          Constantes           * */
/* ********************************* */
 
#pragma mark Constantes
 
enum {
  NGRIDX = 30,
  NGRIDY = 16,
  NMINES = 99,
  OFFGRIDX = 12,
  OFFGRIDY = 75,
  OFFSCORX = 17,
  OFFSCORY = 36
};
 
enum {
  SDL_FLAGS = SDL_INIT_VIDEO,
  WIDTH = NGRIDX * 16 + OFFGRIDX + 8,
  HEIGHT = NGRIDY * 16 + OFFGRIDY + 8,
  BPP = 32,
  SFC_FLAGS = SDL_SWSURFACE,
};
 
typedef enum {
  M_NULL,
   
  M_PART, /* Menu 'Partie'  */
  M_INFO, /* Menu '?' */
   
  M_NEW,  /* Menu 'Nouveau' jeu - F2 */
  M_DEB,  /* Menu 'Débutant' */
  M_INT,  /* Menu 'Intermédiaire' */
  M_EXP,  /* Menu 'Expert' */
  M_PERSO, /* Menu 'Personnalisé' */
  M_QUIT  /* Menu 'Quitter' */
   
} menu_e;
 
enum  {
  REVEAL = 0,
  UNREV = 9,
  FLAG = 10,
  QUEST = 11,
  QUESTP = 12,
  BOMB = 13,
  BOOM = 14,
  WBOMB = 15
};
 
enum {
  OVER0,  /* Aucune case survolée */
  OVER00, /* Clic gauche enfoncé sur la grille puis déplacé en dehors */
  OVER1,  /* Clic gauche */
  OVER9,  /* Clic gauche + droit */
  OVERS0, /* Clic smiley */
  OVERS1  /* Clic smiley + déplacé en dehors */
};
 
enum {
  ST_STOP,
  ST_RUN,
  ST_WIN,
  ST_LOSE
};
 
enum {
  FPS = 50,
  FPS_Default = 1000 / FPS,
  DOUBLECLIC = 10
};
 
typedef Sint32 (*pFct)(void *);
 
/* ********************************* */
/* *          Structures           * */
/* ********************************* */
 
#pragma mark Structures
 
typedef struct events_s {
  Uint8 * pKeys;
  Uint32 nMouseX;
  Uint32 nMouseY;
  Uint8 nMouseButton[3];
  Uint8 nMouseButtonUsed[3];
  Sint32 nDoubleClic;
} events_t;
 
struct gVars_s {
  SDL_Surface * pScreen;
  Uint32 nScrW;
  Uint32 nScrH;
  events_t ev;
  Uint32 nWidth;
  Uint32 nHeight;
};
 
typedef struct timing_s {
  Uint8 nStart;
  Uint8 nTics;
  Uint32 nSec;
} timing_t;
 
typedef struct game_s {
  Uint8 * pGrid;      /* Grille */
  Uint8 * pGridCover; /* Grille affichée */
  timing_t tics;      /* Temps de jeu */
   
  Uint32 nGridW;      /* Nombre de mines en largeur */
  Uint32 nGridH;      /* Nombre de mines en hauteur */
  Uint32 nGridBomb;   /* Nombre de mines dans le jeu */
   
  Uint32 nGameTarget; /* Nombre de cases à découvrir */
  Sint32 nGameUnrev;  /* Nombre de cases non révélées */
  Sint32 nGameBomb;   /* Nombre de mines flaggées */
  Uint8 nState;       /* Etat du jeu : Stop/En cours/Gagné/Perdu */
   
  SDL_Rect rOver;     /* Position centrale du tableau */
  Uint8 pOver[9];     /* Tableau de 'survol' */
  Uint8 nOver;        /* Nombre de cases surovlées */
} game_t;
 
/* ********************************* */
/* *           Globales            * */
/* ********************************* */
 
#pragma mark Globales
 
struct gVars_s gVars;
SDL_Surface * gSprCase;
SDL_Surface * gSprSmiley;
SDL_Surface * gSprDigit;
Uint32 gnTimer1, gnTimer2;
 
 
/* ********************************* */
/* *           Fonctions           * */
/* ********************************* */
 
/* Merci joe78 */
void FrameInit(void) {
  gnTimer1 = SDL_GetTicks();
}
 
void FrameWait(void) {
  while (1) {
    gnTimer2 = SDL_GetTicks() - gnTimer1;
    if (gnTimer2 >= FPS_Default) break;
    SDL_Delay(3);
  }
  gnTimer1 = SDL_GetTicks();
}
 
 
 
SDL_Surface * loadBMP(char * s) {
  SDL_Surface * sfc;
  SDL_Surface * tmp;
   
  if (s == NULL) {
    fprintf(stderr, "loadBMP : Chaine vide.\n");
    exit(EXIT_FAILURE);
  }
  sfc = SDL_LoadBMP(s);
  if (sfc == NULL) {
    fprintf(stderr, "loadBMP : %s.\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
   
  tmp = SDL_DisplayFormat(sfc);
  SDL_FreeSurface(sfc);
  if (tmp == NULL) {
    fprintf(stderr, "loadBMP : %s.\n", SDL_GetError());
    exit(EXIT_FAILURE);
  }
   
  return tmp;
}
 
 
 
 
 
void drawBack(game_t * game) {
  SDL_Rect r;
  Uint32 n0xD7D3CE = SDL_MapRGB(gVars.pScreen->format, 0xD7, 0xD3, 0xCE);
  Uint32 n0xFFFFFF = SDL_MapRGB(gVars.pScreen->format, 0xFF, 0xFF, 0xFF);
  Uint32 n0xC6C3C6 = SDL_MapRGB(gVars.pScreen->format, 0xC6, 0xC3, 0xC6);
  Uint32 n0x848284 = SDL_MapRGB(gVars.pScreen->format, 0x84, 0x82, 0x84);
   
  /* 20px pour le cadre de menus */
  r.x = 0;
  r.y = 0;
  r.w = gVars.nWidth;
  r.h = 20;
  SDL_FillRect(gVars.pScreen, &r, n0xD7D3CE);
   
  /* Cadre total (pour les bordures blanches) */
  r.x = 0;
  r.y = 20;
  r.w = gVars.nWidth;
  r.h = gVars.nHeight;
  SDL_FillRect(gVars.pScreen, &r, n0xFFFFFF);
   
  /* Cadre total (pour le gris) */
  r.x = 3;
  r.y = 23;
  r.w = gVars.nWidth;
  r.h = gVars.nHeight;
  SDL_FillRect(gVars.pScreen, &r, n0xC6C3C6);
   
  /* Cadre du haut (cadre gris foncé) */
  r.x = 9;
  r.y = 29;
  r.w = game->nGridW * gSprCase->h + 6;
  r.h = 37;
  SDL_FillRect(gVars.pScreen, &r,n0x848284);
   
  /* Cadre du haut (cadre blanc) */
  r.x = 11;
  r.y = 31;
  r.w = game->nGridW * gSprCase->h + 4;
  r.h = 35;
  SDL_FillRect(gVars.pScreen, &r, n0xFFFFFF);
   
  /* Cadre du haut (cadre gris clair intérieur) */
  r.x = 11;
  r.y = 31;
  r.w = game->nGridW * gSprCase->h + 2;
  r.h = 33;
  SDL_FillRect(gVars.pScreen, &r, n0xC6C3C6);
   
  /* Cadre du nombre de mines */
  r.x = OFFSCORX - 1;
  r.y = OFFSCORY - 1;
  r.w = 40;
  r.h = 24;
  SDL_FillRect(gVars.pScreen, &r, n0x848284);
   
  r.x = OFFSCORX;
  r.y = OFFSCORY;
  r.w = 40;
  r.h = 24;
  SDL_FillRect(gVars.pScreen, &r, n0xFFFFFF);
   
  /* Cadre du temps */
  r.x = gVars.nScrW - 55;
  r.y = OFFSCORY - 1;
  r.w = 40;
  r.h = 24;
  SDL_FillRect(gVars.pScreen, &r, n0x848284);
   
  r.x = gVars.nScrW - 54;
  r.y = OFFSCORY;
  r.w = 40;
  r.h = 24;
  SDL_FillRect(gVars.pScreen, &r, n0xFFFFFF);
   
  /* Cadre du bas */
  r.x = OFFGRIDX - 3;
  r.y = OFFGRIDY - 3;
  r.w = game->nGridW * gSprCase->h + 6;
  r.h = game->nGridH * gSprCase->h + 6;
  SDL_FillRect(gVars.pScreen, &r, n0x848284);
   
  r.x = OFFGRIDX;
  r.y = OFFGRIDY;
  r.w = game->nGridW * gSprCase->h + 3;
  r.h = game->nGridH * gSprCase->h + 3;
  SDL_FillRect(gVars.pScreen, &r, n0xFFFFFF);
   
  SDL_BlitSurface(gVars.pScreen, NULL, gVars.pScreen, NULL);
}
 
void drawDigit(Sint32 n, SDL_Rect * s) {
  SDL_Rect r;
  char str[4];
  Uint32 i;
   
  r.y = 0;
  r.w = 13;
  r.h = 26;
   
  if (n < -99)
    n = -99;
  else if (n > 999)
    n = 999;
   
  sprintf(str, "%03d", n);
   
  for (i = 0; i < 3; i++) {
    if (str[i] == '-')
      r.x = 10 * r.w;
    else
      r.x = (str[i] - '0') * r.w;
     
    SDL_BlitSurface(gSprDigit, &r, gVars.pScreen, s);
    s->x += r.w;
  }
}
 
void drawTxtTime(game_t * game) {
  SDL_Rect r;
  Sint32 n = game->tics.nSec;
   
  r.x = gVars.nScrW - 54;
  r.y = OFFSCORY;
  r.w = 13;
  r.h = 26;
   
  drawDigit(n, &r);
}
 
void drawTxtMines(game_t * game) {
  SDL_Rect r;
  Sint32 n = game->nGameBomb;
   
  r.x = OFFSCORX;
  r.y = OFFSCORY;
  r.w = 13;
  r.h = 26;
   
  drawDigit(n, &r);
}
 
void drawSmiley(game_t * game) {
  SDL_Rect r, s;
   
  s.x = gVars.nScrW / 2 - gSprSmiley->h / 2 + 2;
  s.y = OFFSCORY - 1;
  s.w = 0;
  s.h = 0;
   
  r.y = 0;
  r.w = gSprSmiley->h;
  r.h = gSprSmiley->h;
   
  if (game->nState == ST_WIN && game->nOver != OVERS0)
    r.x = r.w * 3;
  else if (game->nState == ST_LOSE && game->nOver != OVERS0)
    r.x = r.w * 2;
  else {
    if (game->nOver == OVERS0)
      r.x = r.w * 4;
    else if (game->nOver == OVERS1)
      r.x = 0;
    else if ((gVars.ev.nMouseButton[0] && gVars.ev.nMouseButtonUsed[0] == 0) ||
             (gVars.ev.nMouseButton[0] && gVars.ev.nMouseButton[1] &&
              gVars.ev.nMouseButtonUsed[0] == 1)) {
               r.x = r.w * 1;
             }
    else
      r.x = 0;
  }
   
  SDL_BlitSurface(gSprSmiley, &r, gVars.pScreen, &s);
}
 
 
 
 
 
 
 
 
 
Sint32 eventHandler(void) {
  SDL_Event event;
   
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_KEYDOWN:
        if (gVars.ev.pKeys[SDLK_ESCAPE])
          return 1;
        break;
         
      case SDL_KEYUP:
        break;
         
      case SDL_QUIT:
        return 1;
        break;
         
      case SDL_MOUSEMOTION:
        gVars.ev.nMouseX = event.motion.x;
        gVars.ev.nMouseY = event.motion.y;
        break;
         
      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT) {
          gVars.ev.nMouseButton[0] = 1;
        }
        else if (event.button.button == SDL_BUTTON_RIGHT) {
          gVars.ev.nMouseButton[1] = 1;
        }
        else if (event.button.button == SDL_BUTTON_MIDDLE) {
          gVars.ev.nMouseButton[2] = 1;
        }
        break;
         
      case SDL_MOUSEBUTTONUP:
        if (event.button.button == SDL_BUTTON_LEFT) {
          gVars.ev.nMouseButton[0] = 0;
          gVars.ev.nMouseButtonUsed[0] = 0;
        }
        else if (event.button.button == SDL_BUTTON_RIGHT) {
          gVars.ev.nMouseButton[1] = 0;
          gVars.ev.nMouseButtonUsed[1] = 0;
        }
        else if (event.button.button == SDL_BUTTON_MIDDLE) {
          gVars.ev.nMouseButton[2] = 0;
          gVars.ev.nMouseButtonUsed[2] = 0;
        }
        break;
         
      default:
        break;
    }
  }
   
  return 0;
}
 
void incBomb(game_t * game, Sint32 x, Sint32 y) {
  Sint32 i, j;
   
  for (i = -1; i <= 1; i++) {
    for (j = -1; j <= 1; j++) {
      if (i == 0 && j == 0)
        continue;
      if (x + i >= 0 && x + i < game->nGridW &&
          y + j >= 0 && y + j < game->nGridH &&
          game->pGrid[(y + j) * game->nGridW + (x + i)] != BOMB) {
        game->pGrid[(y + j) * game->nGridW + (x + i)]++;
      }
    }
  }
}
 
void initGrid(game_t * game) {
  Sint32 i, j, n;
  Sint32 sz = game->nGridH * game->nGridW;
  Sint32 * pTab;
   
  pTab = malloc(sz * sizeof *pTab);
  if (pTab == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
   
  for (i = 0; i < sz; i++)
    pTab[i] = i;
  for (i = sz - 1; i > 0; i--) {
    n = rand() % i;
     
    j = pTab[i];
    pTab[i] = pTab[n];
    pTab[n] = j;
  }
   
  memset(game->pGrid, 0, sz);
  for (n = 0; n < game->nGridBomb; n++) {
    i = pTab[n];
    game->pGrid[i] = BOMB;
    incBomb(game, i % game->nGridW, i / game->nGridW);
  }
   
  for (i = 0; i < sz; i++)
    game->pGridCover[i] = UNREV;
   
  free(pTab);
}
 
 
 
Sint32 gameInit(void * args) {
  game_t * this = args;
   
  this->nGridW = NGRIDX;
  this->nGridH = NGRIDY;
  this->nGridBomb = NMINES;
   
  this->nGameUnrev = this->nGridH * this->nGridW;
  this->nGameBomb = NMINES;
  this->nGameTarget = this->nGameUnrev - this->nGameBomb;
  this->nState = ST_STOP;
   
  this->pGrid = malloc(sizeof *this->pGrid * this->nGridH * this->nGridW);
  if (this->pGrid == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  this->pGridCover = malloc(sizeof *this->pGrid * this->nGridH * this->nGridW);
  if (this->pGrid == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
   
  initGrid(this);
   
  memset(this->pOver, 0, 9);
  this->nOver = OVER0;
   
  this->tics.nSec = 0;
  this->tics.nStart = 0;
  this->tics.nTics = 0;
   
  drawBack(this);
   
  return 0;
}
 
 
Sint32 gameRelease(void * args) {
  game_t * this = args;
   
  free(this->pGrid);
   
  return 0;
}
 
Sint32 gameDraw(void * args) {
  game_t * this = args;
  SDL_Rect r, s;
  Uint32 i, j;
  Uint32 offsetX, offsetY;
   
  offsetX = OFFGRIDX;
  offsetY = OFFGRIDY;
   
  for (i = 0; i < this->nGridH; i++) {
    for (j = 0; j < this->nGridW; j++) {
      r.x = gSprCase->h * this->pGridCover[i * this->nGridW + j];
      r.y = 0;
      r.w = gSprCase->h;
      r.h = gSprCase->h;
       
      s.x = gSprCase->h * j + offsetX;
      s.y = gSprCase->h * i + offsetY;
      s.w = 0;
      s.h = 0;
       
      SDL_BlitSurface(gSprCase, &r, gVars.pScreen, &s);
    }
  }
   
   
  if (this->nOver == OVER1) {
    r.x = gSprCase->h * this->pOver[0];
    r.y = 0;
    r.w = gSprCase->h;
    r.h = gSprCase->h;
     
    s.x = gSprCase->h * this->rOver.x + offsetX;
    s.y = gSprCase->h * this->rOver.y + offsetY;
    s.w = 0;
    s.h = 0;
     
    SDL_BlitSurface(gSprCase, &r, gVars.pScreen, &s);
  }
  else if (this->nOver == OVER9) {
    Sint32 i, j, k;
    Sint32 tmpx, tmpy;
     
    k = 0;
    for (i = -1; i <= 1; i++) {
      for (j = -1; j <= 1; j++) {
        r.x = gSprCase->h * this->pOver[k];
        r.y = 0;
        r.w = gSprCase->h;
        r.h = gSprCase->h;
         
        tmpx = this->rOver.x + i;
        tmpy = this->rOver.y + j;
         
        if (tmpx >= 0 && tmpx < this->nGridW &&
            tmpy >= 0 && tmpy < this->nGridH) {
          s.x = gSprCase->h * tmpx + offsetX;
          s.y = gSprCase->h * tmpy + offsetY;
          s.w = 0;
          s.h = 0;
           
          SDL_BlitSurface(gSprCase, &r, gVars.pScreen, &s);
        }
         
        k++;
      }
    }
  }
   
  drawSmiley(this);
   
  drawTxtMines(this);
  drawTxtTime(this);
   
  return 0;
}
 
 
void incTime(game_t * game) {
  if (game->tics.nStart) {
    game->tics.nTics++;
     
    if (game->tics.nTics > FPS) {
      game->tics.nSec++;
      game->tics.nTics = 0;
    }
     
    if (game->tics.nSec > 999) {
      game->tics.nSec = 999;
    }
  }
}
 
 
 
Uint8 coverToOver(Uint8 val) {
  if (val == UNREV)
    val = REVEAL;
  else if (val == QUEST)
    val = QUESTP;
   
  return val;
}
 
void gameSetFlag(game_t * game, SDL_Rect * r) {
  Uint32 tmp;
   
  tmp = r->y * game->nGridW + r->x;
   
  if (game->pGridCover[tmp] == UNREV) {
    game->pGridCover[tmp] = FLAG;
    game->nGameBomb--;
  }
  else if (game->pGridCover[tmp] == FLAG) {
    game->pGridCover[tmp] = QUEST;
    game->nGameBomb++;
  }
  else if (game->pGridCover[tmp] == QUEST)
    game->pGridCover[tmp] = UNREV;
   
}
 
void gameSetCover(game_t * game, SDL_Rect * r, Uint8 nOver) {
  Sint32 tmpi;
   
  if (nOver == OVER1) {
    game->nOver = OVER1;
     
    tmpi = r->y * game->nGridW + r->x;
    game->pOver[0] = coverToOver(game->pGridCover[tmpi]);
    game->rOver = *r;
  }
  else if (nOver == OVER9) {
    Sint32 i, j, k;
    Sint32 tmpx, tmpy;
     
    game->nOver = OVER9;
    game->rOver = *r;
     
    k = 0;
    for (i = -1; i <= 1; i++) {
      for (j = -1; j <= 1; j++) {
        tmpx = r->x + i;
        tmpy = r->y + j;
         
        if (tmpx >= 0 && tmpx < game->nGridW &&
            tmpy >= 0 && tmpy < game->nGridH) {
          tmpi = tmpy * game->nGridW + tmpx;
          game->pOver[k] = coverToOver(game->pGridCover[tmpi]);
        }
        else {
          game->pOver[k] = REVEAL;
        }
         
        k++;
      }
    }
  }
}
 
void gameRevealCaseUnrev(game_t * game, const SDL_Rect * r) {
  Sint32 i, j;
  SDL_Rect s;
  Sint32 tmp;
   
  for (i = -1; i <= 1; i++) {
    for (j = -1; j <= 1; j++) {
      if (i == 0 && j == 0) {
        continue;
      }
       
      s.x = r->x + i;
      s.y = r->y + j;
      tmp = s.y * game->nGridW + s.x;
       
      /* Si on est dans le tableau et que la case n'est pas dévoilée et qu'on
       * a pas flaggé la case */
      if (s.x >= 0 && s.x < game->nGridW &&
          s.y >= 0 && s.y < game->nGridH &&
          (game->pGridCover[tmp] == UNREV || game->pGridCover[tmp] == QUEST ||
           game->pGridCover[tmp] == QUESTP)) {
             
            game->pGridCover[tmp] = game->pGrid[tmp];
            game->nGameTarget--;
             
            if (game->pGridCover[tmp] == REVEAL) {
              gameRevealCaseUnrev(game, &s);
            }
          }
    }
  }
}
 
 
/* On considère que le test de la case a déjà été fait avant d'appeler
 * la fonction. */
Sint32 gameReveal1Case(game_t * game, const SDL_Rect * r) {
  Uint32 tmp;
   
  tmp = r->y * game->nGridW + r->x;
   
  /* On découvre la/les cases */
  if (game->pGridCover[tmp] != FLAG) {
    if (game->pGridCover[tmp] == UNREV ||
        game->pGridCover[tmp] == QUEST ||
        game->pGridCover[tmp] == QUESTP) {
      game->nGameTarget--;
    }
     
    if (game->pGrid[tmp] == REVEAL && game->pGridCover[tmp] != REVEAL) {
      game->pGridCover[tmp] = game->pGrid[tmp];
      gameRevealCaseUnrev(game, r);
    }
    else if (game->pGrid[tmp] == BOMB) {
      Uint32 i;
       
      for (i = 0; i < game->nGridH * game->nGridW; i++) {
        if (game->pGridCover[i] == FLAG && game->pGrid[i] != BOMB) {
          game->pGridCover[i] = WBOMB;
        }
        else if (game->pGrid[i] == BOMB) {
          game->pGridCover[i] = BOMB;
        }
      }
       
      game->nGameTarget = 1;
      game->pGridCover[tmp] = BOOM;
      game->tics.nStart = 0;
      game->nState = ST_LOSE;
       
      return -1;
    }
    else {
      game->pGridCover[tmp] = game->pGrid[tmp];
    }
  }
   
  return 0;
}
 
/* On considère que le test de la case a déjà été fait avant d'appeler
 * la fonction. */
void gameReveal9Case(game_t * game, const SDL_Rect * r) {
  SDL_Rect s;
  Sint32 i, j;
  Sint32 tmp;
  Sint32 nFlag;
   
  tmp = r->y * game->nGridW + r->x;
   
  if (game->pGridCover[tmp] <= REVEAL || game->pGridCover[tmp] >= UNREV)
    return;
   
  /* On compte le nombre de flag alentour */
  nFlag = 0;
  for (i = -1; i <= 1; i++) {
    for (j = -1; j <= 1; j++) {
      if (i == 0 && j == 0) {
        continue;
      }
       
      s.x = r->x + i;
      s.y = r->y + j;
       
      if (s.x >= 0 && s.x < game->nGridW &&
          s.y >= 0 && s.y < game->nGridH &&
          game->pGridCover[s.y * game->nGridW + s.x] == FLAG) {
        nFlag++;
      }
    }
  }
   
  /* Si le nombre de flag autour est égal au nombre indiqué sur la case */
  if (nFlag == game->pGridCover[tmp]) {
    for (i = -1; i <= 1; i++) {
      for (j = -1; j <= 1; j++) {
        if (i == 0 && j == 0) {
          continue;
        }
         
        s.x = r->x + i;
        s.y = r->y + j;
         
        if (s.x >= 0 && s.x < game->nGridW && s.y >= 0 && s.y < game->nGridH &&
            gameReveal1Case(game, &s) < 0)
          return;
      }
    }
  }
}
 
Sint32 checkDoubleClic(game_t * game) {
  /* On démarre le compteur */
  if (gVars.ev.nDoubleClic < 0) {
    gVars.ev.nDoubleClic = 0;
  }
  else if (gVars.ev.nDoubleClic >= 0 && gVars.ev.nDoubleClic <= DOUBLECLIC) {
    return 1;
  }
  else {
    gVars.ev.nDoubleClic = -1;
  }
   
  return 0;
}
 
Sint32 gameMouse(game_t * game) {
  SDL_Rect r;
  Uint8 nState = game->nState != ST_LOSE && game->nState != ST_WIN;
   
  r.x = (gVars.ev.nMouseX - OFFGRIDX) / gSprCase->h;
  r.y = (gVars.ev.nMouseY - OFFGRIDY) / gSprCase->h;
  r.w = 0;
  r.h = 0;
   
   
  /* Appui sur les 2 boutons */
  if (nState && ((gVars.ev.nMouseButton[0] && gVars.ev.nMouseButton[1]) ||
                 gVars.ev.nMouseButton[2])) {
    if (gVars.ev.nMouseButton[2] == 0)
      gVars.ev.nMouseButtonUsed[1] = 1;
     
    /* Si on est en dehors des limites, on remet la/les cases */
    if (r.x < 0 || r.x >= game->nGridW || r.y < 0 || r.y >= game->nGridH) {
      game->nOver = OVER0;
    }
    else {
      gameSetCover(game, &r, OVER9);
    }
  }
  /* Bouton gauche relâché en étant appuyé sur le bouton droit*/
  else if (gVars.ev.nMouseButton[0] == 0 && gVars.ev.nMouseButton[1] &&
           game->nOver == OVER9) {
    /* Si on est dans les limites */
    if (nState &&
        r.x >= 0 && r.x < game->nGridW && r.y >= 0 && r.y < game->nGridH) {
       
      game->tics.nStart = 1;
      game->nState = ST_RUN;
       
      if (gVars.ev.nDoubleClic < 0) {
        gVars.ev.nDoubleClic = 0;
      }
       
      gameReveal9Case(game, &r);
      game->nOver = OVER0;
    }
  }
  /* Appui sur le bouton gauche */
  else if (gVars.ev.nMouseButton[0] && gVars.ev.nMouseButtonUsed[0] == 0) {
     
    /* Appui sur le smiley */
    if (game->nOver == OVERS0 || game->nOver == OVERS1) {
      Uint32 tmpx, tmpy;
       
      tmpx = gVars.nScrW / 2 - gSprSmiley->h / 2 + 2;
      tmpy = OFFSCORY - 1;
       
      if (gVars.ev.nMouseX >= tmpx && gVars.ev.nMouseX < tmpx + gSprSmiley->h &&
          gVars.ev.nMouseY >= tmpy && gVars.ev.nMouseY < tmpy + gSprSmiley->h) {
        game->nOver = OVERS0;
      }
      else {
        game->nOver = OVERS1;
      }
    }
    /* Si on est en dehors des limites, on remet la/les cases */
    else if (r.x < 0 || r.x >= game->nGridW || r.y < 0 || r.y >= game->nGridH) {
      /* Pour 'capturer' le smiley */
      if (game->nOver == OVER0) {
        Uint32 tmpx, tmpy;
         
        tmpx = gVars.nScrW / 2 - gSprSmiley->h / 2 + 2;
        tmpy = OFFSCORY - 1;
         
        if (gVars.ev.nMouseX >= tmpx && gVars.ev.nMouseX < tmpx + gSprSmiley->h &&
            gVars.ev.nMouseY >= tmpy && gVars.ev.nMouseY < tmpy + gSprSmiley->h) {
          game->nOver = OVERS0;
        }
        else {
          game->nOver = OVER00;
        }
      }
      else if (game->nOver == OVER1 || game->nOver == OVER9 ||
               game->nOver == OVER00) {
        game->nOver = OVER00;
      }
      else {
        game->nOver = OVER0;
      }
    }
    /* Si on était en train d'appuyer sur les 2 boutons, on remet les cases
     * Ca veut dire qu'on a relâché le bouton gauche
     */
    else if (game->nOver == OVER9) {
      game->nOver = OVER0;
      gVars.ev.nMouseButtonUsed[0] = 1;
    }
    /* Sinon, on vient juste d'appuyer sur le bouton gauche ou alors on
     * update la position */
    else if (nState &&
             r.x >= 0 && r.x < game->nGridW &&
             r.y >= 0 && r.y < game->nGridH) {
      gameSetCover(game, &r, OVER1);
    }
  }
  /* Si on relâche le clic droit en maintenant le clic gauche */
  else if (nState && gVars.ev.nMouseButton[1] && game->nOver == OVER9) {
    /* On réinitialise le 'survol' */
    game->nOver = OVER0;
    game->tics.nStart = 1;
  }
  /* Si on avait appuyé sur le bouton gauche puis relâché */
  else if (gVars.ev.nMouseButton[0] == 0 && game->nOver == OVER1) {
    /* Si on est dans les limites */
    if (nState &&
        r.x >= 0 && r.x < game->nGridW && r.y >= 0 && r.y < game->nGridH) {
       
      game->tics.nStart = 1;
      game->nState = ST_RUN;
       
      /* Double clic sur une case révélée avec un numéro */
      /* On vérifie que le double clic se fasse sur la même case... */
      if (checkDoubleClic(game) &&
          game->pGridCover[r.y * game->nGridW + r.x] > REVEAL &&
          game->pGridCover[r.y * game->nGridW + r.x] < UNREV &&
          game->rOver.x == r.x && game->rOver.y == r.y) {
        gameReveal9Case(game, &r);
      }
      else {
        gameReveal1Case(game, &r);
         
        game->nOver = OVER0;
      }
    }
  }
  /* Si on avait appuyé sur le bouton gauche puis relâché sur le smiley */
  else if (gVars.ev.nMouseButton[0] == 0 && game->nOver == OVERS0) {
    Uint32 tmpx, tmpy;
     
    tmpx = gVars.nScrW / 2 - gSprSmiley->h / 2 + 2;
    tmpy = OFFSCORY - 1;
     
    if (gVars.ev.nMouseX >= tmpx && gVars.ev.nMouseX < tmpx + gSprSmiley->h &&
        gVars.ev.nMouseY >= tmpy && gVars.ev.nMouseY < tmpy + gSprSmiley->h) {
      return 1;
    }
  }
  /* Si on appuie pour la première fois sur le bouton droit */
  else if (nState &&
           gVars.ev.nMouseButton[1] && gVars.ev.nMouseButtonUsed[1] == 0) {
    /* Si on est dans les limites on flag */
    if (r.x >= 0 && r.x < game->nGridW && r.y >= 0 && r.y < game->nGridH) {
      gVars.ev.nMouseButtonUsed[1] = 1;
      gameSetFlag(game, &r);
    }
  }
  /* Aucun bouton appuyé */
  else if (gVars.ev.nMouseButton[0] == 0 && gVars.ev.nMouseButton[1] == 0 &&
           gVars.ev.nMouseButton[2] == 0) {
     
    /* Pas besoin de tester si on est dans les limites, si on est en
     * OVER9 c'est qu'on l'était forcément */
    if (game->nOver == OVER9) {
      gameReveal9Case(game, &game->rOver);
    }
     
    game->nOver = OVER0;
  }
   
  return 0;
}
 
void checkWin(game_t * game) {
  if (game->nGameTarget <= 0) {
    Uint32 i;
     
    for (i = 0; i < game->nGridH * game->nGridW; i++) {
      if (game->pGrid[i] == BOMB) {
        game->pGridCover[i] = FLAG;
      }
    }
     
    game->nGameBomb = 0;
    game->nState = ST_WIN;
    game->tics.nStart = 0;
  }
}
 
void incDoubleClic(game_t * game) {
  if (gVars.ev.nDoubleClic > DOUBLECLIC) {
    gVars.ev.nDoubleClic = -1;
  }
  else if (gVars.ev.nDoubleClic >= 0) {
    gVars.ev.nDoubleClic++;
  }
}
 
Sint32 gameMain(void * args) {
  game_t * this = args;
   
  if (gVars.ev.pKeys[SDLK_F2]) {
    gVars.ev.pKeys[SDLK_F2] = 0;
    return M_NEW;
  }
   
  if (gameMouse(this) == 1)
    return M_NEW;
  checkWin(this);
  incDoubleClic(this);
  incTime(this);
   
  return M_NULL;
}
 
 
 
 
 
 
menu_e menu(pFct pFctInit, pFct pFctMain, pFct pFctRelease, pFct pFctDraw, void * args) {
  menu_e nMenu;
   
  if (pFctInit(args) < 0)
    return M_QUIT;
   
  FrameInit();
  nMenu = M_NULL;
  while (nMenu == M_NULL) {
    if (eventHandler() != 0) {
      nMenu = M_QUIT;
      break;
    }
     
    nMenu = pFctMain(args);
    pFctDraw(args);
     
    FrameWait();
    SDL_Flip(gVars.pScreen);
  }
   
  pFctRelease(args);
   
  return nMenu;
}
 
void mainLoop(void) {
  game_t game;
  menu_e nState;
   
  nState = M_NULL;
  while (nState != M_QUIT) {
    switch (nState) {
      case M_NULL:
        nState = menu(gameInit, gameMain, gameRelease, gameDraw, &game);
        break;
         
      case M_NEW:
        nState = menu(gameInit, gameMain, gameRelease, gameDraw, &game);
        break;
         
        /* Lancement du menu 'Partie' */
      case M_PART:
         
        break;
         
        /* Lancement du menu '?' */
      case M_INFO:
         
        break;
         
      default:
        fprintf(stderr, "Option inconnue.\n");
        nState = M_NULL;
        break;
    }
  }
}
 
 
 
 
 
 
 
 
 
 
void initGVars(void) {
  gVars.pScreen = SDL_GetVideoSurface();
  gVars.nScrW = WIDTH;
  gVars.nScrH = HEIGHT;
  gVars.nWidth = WIDTH;
  gVars.nHeight = HEIGHT;
  gVars.ev.pKeys = SDL_GetKeyState(NULL);
  gVars.ev.nMouseX = -1;
  gVars.ev.nMouseY = -1;
  gVars.ev.nMouseButton[0] = 0;
  gVars.ev.nMouseButton[1] = 0;
  gVars.ev.nMouseButtonUsed[0] = 0;
  gVars.ev.nMouseButtonUsed[1] = 0;
  gVars.ev.nDoubleClic = -1;
   
  gSprCase = loadBMP("cases.bmp");
  gSprDigit = loadBMP("digits.bmp");
  gSprSmiley = loadBMP("smileys.bmp");
}
 
Sint32 init(void) {
  if (SDL_Init(SDL_FLAGS) < 0) {
    fprintf(stderr, "SDL_Init : %s\n", SDL_GetError());
    return -1;
  }
  atexit(SDL_Quit);
   
  if (SDL_SetVideoMode(WIDTH, HEIGHT, BPP, SFC_FLAGS) == NULL) {
    fprintf(stderr, "SDL_SetVideoMode : %s\n",
            SDL_GetError());
    return -1;
  }
   
  SDL_WM_SetCaption("Démineur", NULL);
  srand(time(NULL));
  initGVars();
   
  return 0;
}
 
int main(int argc, char ** argv) {
  if (init() < 0)
    return EXIT_FAILURE;
   
   
  mainLoop();
  return EXIT_SUCCESS;
}

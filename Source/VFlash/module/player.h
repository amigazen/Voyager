#ifndef VFLASH_PLAYER_H
#define VFLASH_PLAYER_H
/*
 * VFlash portable player functions
 * --------------------------------
 *
 * $Id: player.h,v 1.1.1.1 2002/10/07 12:52:14 zapek Exp $
 *
 */


#if DEBUG
void draw_info(void);
#endif
void play_movie(FlashHandle flashHandle);
void showUrl(char *url, char *target, void *udata);
void getSwf(char *url, int level, void *client_data);

#endif /* !VFLASH_PLAYER_H */

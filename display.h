
extern void cr(void);

extern void __fastcall__ printCbm(char *str);
extern void __fastcall__ printCbm_cr(char *str);
extern void __fastcall__ printAsc(char *str);
extern void __fastcall__ printAsc_cr(char *str);
extern void __fastcall__ printInt(int x);

extern void propfont_setup(void);
extern void propfont_cls(void);
extern void propfont_clearline(void);
extern void propfont_startstatus(void);
extern void propfont_endstatus(void);
extern char propfont_getc(void);
extern char propfont_getx(void);
extern void __fastcall__ propfont_setcol(char col);
extern void __fastcall__ propfont_putc(char c);
extern void __fastcall__ propfont_unputc(char c);

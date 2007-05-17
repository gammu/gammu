
size_t		UnicodeLength			(const unsigned char *str);
unsigned char	*DecodeUnicodeString		(const unsigned char *src);
unsigned char   *DecodeUnicodeConsole		(const unsigned char *src);
void		DecodeUnicode			(const unsigned char *src, unsigned char *dest);
void		EncodeUnicode			(unsigned char *dest, const char *src, int len);
void 		ReadUnicodeFile			(unsigned char *Dest, unsigned char *Source);

void 		CopyUnicodeString		(unsigned char *Dest, unsigned char *Source);


size_t		UnicodeLength			(const unsigned char *str);
unsigned char	*DecodeUnicodeString		(const unsigned char *src);
unsigned char   *DecodeUnicodeConsole		(const unsigned char *src);
void		DecodeUnicode			(const unsigned char *src, unsigned char *dest);
void		EncodeUnicode			(unsigned char *dest, const char *src, int len);
void 		ReadUnicodeFile			(unsigned char *Dest, unsigned char *Source);

void 		CopyUnicodeString		(unsigned char *Dest, unsigned char *Source);

bool		EncodeUTF8QuotedPrintable	(unsigned char *dest, const unsigned char *src);
void 		DecodeUTF8QuotedPrintable	(unsigned char *dest, const unsigned char *src, int len);
int 		EncodeWithUTF8Alphabet2		(unsigned char mychar1, unsigned char mychar2, unsigned char *ret);
int 		DecodeWithUTF8Alphabet2		(unsigned char *src, wchar_t *dest, int len);


/* 
 * Decompress the fat on the disk
 * img: address of the compressed FAT
 * fat: buffer to store the decompressed fat
 */
void file_readfat(int *fat, unsigned char *img)
{
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) { // 2880 secters
		fat[i + 0] = (img[j + 0]      | img[j + 1] << 8) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | img[j + 2] << 4);
		j += 3;
	}
	return;
}

/* 
 * Read the file into *buf
 * clustno: the first cluster number
 * size: the size of the file
 * buf: a buffer for the file
 * fat: a decompressed FAT
 * img: address of the starting point of disk files
 */
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img)
{
	int i;
	while (1) {
		if (size <= 512) {
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];
	}
	return;
}
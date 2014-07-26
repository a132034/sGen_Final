#include	"dy_lib.h"

INT Read_Char_For_P(std::ifstream& file);
INT Read_Int_For_P(std::ifstream& file);

template <class T>
VOID DCT_1D(T *in, T *out, CONST INT N)
{
	register INT k, n;
	T dct = 0;
	T temp = 0;
	T omega = 0;
	for (k = 1; k <= N; ++k) // 결과 Loop
	{
		dct = 0;
		for (n = 1; n <= N; ++n) // sigma
		{
			if (k == 1)
				omega = 1.0 / sqrt((T)N);
			else // 2 <= k <= N
			{
				omega = 2.0 / N;
				omega = sqrt(omega);
			}

			temp = (PI * (2.0 * n - 1) * (k - 1)) / (2 * N);
			dct += omega * in[n - 1] * cos(temp);
		}
		out[k - 1] = dct;
	}

}

template <class T>
VOID IDCT_1D(T *in, T *out, CONST INT N)
{
	register INT k, n;
	T dct = 0;
	T temp = 0;
	T omega = 0;
	for (n = 1; n <= N; ++n) // 결과 Loop
	{
		dct = 0;
		for (k = 1; k <= N; ++k) // sigma
		{
			if (k == 1)
				omega = 1.0 / sqrt((T)N);
			else // 2 <= k <= N
			{
				omega = 2.0 / N;
				omega = sqrt(omega);
			}

			temp = (PI * (2.0 * n - 1) * (k - 1)) / (2 * N);
			dct += omega * in[k - 1] * cos(temp);
		}
		out[n - 1] = dct;
	}


}

template <class T>
VOID DFT_1D(T* real, T* imagine, CONST INT Length, CONST INT Forward)
{
	register INT i, j, k, n;
	INT N = Length;

	T* tr = new T[sizeof(T)*N]; // temp re 
	T* ti = new T[sizeof(T)*N]; // temp im

	CopyMemory(tr, real, sizeof(T)*N);
	CopyMemory(ti, imagine, sizeof(T)*N);

	T re, im, temp;
	T cosine, sine;

	for (k = 1; k <= N; ++k)
	{
		re = 0.0;
		im = 0.0;
		for (j = 1; j <= N; ++j)
		{
			temp = 2 * Forward*PI*(j - 1)*(k - 1) / (N); //-j
			cosine = cos(temp);
			sine = sin(temp);
			re += (tr[j - 1] * cosine + ti[j - 1] * sine);
			im += (ti[j - 1] * cosine - tr[j - 1] * sine);
		}
		real[k - 1] = re;
		imagine[k - 1] = im;
	}

	if (Forward == -1) // IDFT
	{
		for (i = 0; i < N; i++)
		{
			real[i] /= (T)N;
			imagine[i] /= (T)N;
		}
	}

	delete[] tr;
	delete[] ti;
}

template <class T>
DOUBLE PSNR(T* origin, T* target, CONST INT size, INT bitPerPixel)
{
	DOUBLE psnr = 0.0;
	INT Peak = pow(2, bitPerPixel);
	DOUBLE mse = MSE(origin, target, size);

	psnr = 10.0 * log10((Peak*Peak) / mse);

	return psnr;
}

template <class T>
DOUBLE MSE(T* origin, T* target, CONST INT size)
{
	register INT i;
	DOUBLE mse = 0.0;

	for (i = 0; i < size; ++i)
		mse += (origin[i] - target[i]) * (origin[i] - target[i]);

	mse /= size;

	return mse;
}

template<typename T, typename C>
VOID CLIPPING(T* data, CONST INT size, CONST C min, CONST C max)
{
	register INT i;
	for (i = 0; i < size; ++i)
	{
		if (data[i] > max)
			data[i] = max;
		else if (data[i] < min)
			data[i] = min;
	}
}

template<typename T, typename C>
T ROUND(C value, CONST INT position)
{
	T result;
	result = (T)value * pow(10, position);
	result = floor(result + 0.5);
	result *= pow(10, -position);
	return T;
}

template<typename T, typename C>
T* ROUND(C* value, CONST INT position, CONST INT size)
{
	T* result;
	register INT i;
	for (i = 0; i < size; ++i)
	{
		result[i] = (T)value[i] * pow(10, position);
		result[i] = floor(result + 0.5);
		result[i] *= pow(10, -position);
	}
	return T;
}
template<typename T, typename C>
T* ROUND(C* value, CONST INT position, CONST INT row, CONST INT col)
{
	T* result;
	INT size = row*col;
	register INT i;
	for (i = 0; i < size; ++i)
	{
		result[i] = (T)value[i] * pow(10, position);
		result[i] = floor(result + 0.5);
		result[i] *= pow(10, -position);
	}
	return T;
}


template <typename T>
VOID REVERSE_IMAGE(T* data, CONST INT size, CONST INT bitPerPixel)
{
	register INT i;
	INT max = pow(2, bitPerPixel);
	for (i = 0; i < size; ++i)
		data[i] = max - data[i];

}
template <typename T>
VOID REVERSE_IMAGE(T* data, CONST INT row, CONST INT col, CONST INT bitPerPixel)
{
	INT size = row*col;
	register INT i;
	INT max = pow(2, bitPerPixel);
	for (i = 0; i < size; ++i)
		data[i] = max - data[i];
}

VOID WritePgm(INT row, INT col, BYTE* img, LPCSTR  filename)
{
	std::ofstream file;
	char* header = "P5\n# Created by Vision Image Processing(VIP) Lab, Sangmyung Univ. Korea \n";

	file.open(filename, std::ios::out | std::ios::binary);
	if (file.fail())
	{
		std::cout << "file open error in WritePgm function" << std::endl;
		exit(1);
	}

	//header information writing phase 'P5' is for pgm file format
	file << header << col << ' ' << row << std::endl << "255" << std::endl;
	file.write((char*)img, row*col);

	file.close();

}

BYTE* ReadPgm(INT *row, INT *col, LPCSTR  filename)
{
	std::ifstream file;
	BYTE *buffer, *handle;
	INT size;

	file.open(filename, std::ios::in | std::ios::binary);
	if (file.fail())
	{
		std::cout << "file open error in ReadPgm function" << std::endl;
		exit(1);
	}

	//header information reading phase 'P5' is for pgm file format
	if ((file.get() != 'P') || (file.get() != '5'))
	{
		std::cerr << "error : image format is not a pgm" << std::endl;
		exit(1);
	}

	//read col & row
	*col = Read_Int_For_P(file);
	*row = Read_Int_For_P(file);
	Read_Int_For_P(file);

	size = (*row)*(*col);

	buffer = new byte[size];
	handle = buffer;

	if (buffer == NULL)
	{
		std::cerr << "error: out of memory in ReadPgm function" << std::endl;
		exit(1);
	}

	// read image from file
	file.read((char*)buffer, size);
	file.close();
	return handle;
}


/* FOR PGM, PPM, AND PBM FILE FORMAT */
INT Read_Char_For_P(std::ifstream& file)
{
	INT c;

	c = file.get();
	if (c == '#')
	{
		do{
			c = file.get();
		} while (c != '\n' && c != file.eof());
	}

	return c;
}
INT Read_Int_For_P(std::ifstream& file)
{
	INT c, val;

	do{
		c = Read_Char_For_P(file);
	} while (c == ' ' || c == '\t' || c == '\n');

	val = c - '0';

	while ((c = Read_Char_For_P(file)) >= '0' && c <= '9')
	{
		val *= 10;
		val += c - '0';
	}

	return val;
}
/* END FOR PGM, PPM, AND PBM FILE FORMAT */

VOID WriteBmp(INT row, INT col, BYTE* img, LPCSTR  filename)
{
	register INT i, j;
	std::ofstream file;
	INT m_col;
	INT tempInt;
	WORD tempWord;
	WORD check = 19778; // "BM"
	BYTE *rawdata;

	file.open(filename, std::ios::out | std::ios::binary);
	if (file.fail())
	{
		std::cout << "file open error in WriteBmp function" << std::endl;
		exit(1);
	}

	//type "BM" write
	file.write((char*)&check, 2);

	if ((col * 3) % 4 == 0)
	{
		m_col = col * 3;
	}
	else
	{
		m_col = 4 * ((col * 3) / 4) + 4;
	}

	tempInt = 54 + m_col*row; file.write((char*)&tempInt, 4);		//file size
	tempInt = 0; file.write((char*)&tempInt, 4);					//reserved1
	tempInt = 54; file.write((char*)&tempInt, 4);					//reserved2
	tempInt = 40; file.write((char*)&tempInt, 4);					//offset

	//col & row write
	file.write((char*)&col, 4);
	file.write((char*)&row, 4);

	tempWord = 1; file.write((char*)&tempWord, 2);					//color plane

	// num of bit(24 bit)
	tempWord = 24; file.write((char*)tempWord, 2);

	tempInt = 0; file.write((char*)&tempInt, 4);					//compresstion flag
	tempInt = m_col*row; file.write((char*)&tempInt, 4);			//rawdata size
	tempInt = 3937; file.write((char*)&tempInt, 4);					//width resolution
	tempInt = 3937; file.write((char*)&tempInt, 4);					//height resolution

	// load color num
	tempInt = 0; file.write((char*)&tempInt, 4);

	tempInt = 0; file.write((char*)&tempInt, 4);					//important color

	//load file
	rawdata = new BYTE[row*col * 3];

	//raw data(rgb plane) to bmp format
	for (i = 0; i < row; ++i) {
		for (j = 0; j < col; ++j) {
			rawdata[3 * ((row - i - 1)*col + j) + 2] = img[i*col + j];
			rawdata[3 * ((row - i - 1)*col + j) + 1] = img[i*col + j + col*row];
			rawdata[3 * ((row - i - 1)*col + j)] = img[i*col + j + col*row * 2];
		}
	}

	//rgb0 rgb0 ... 
	for (i = 0; i < row; ++i) {
		file.write((char*)(rawdata + i * col * 3), col * 3);
		if (m_col != (col * 3)) {
			for (j = 0; j < (m_col - col * 3); j++)	file.put(0);
		}
	}

	delete[]rawdata;
	file.close();
}

BYTE* ReadBmp(INT *row, INT *col, LPCSTR  filename)
{
	register INT i, j;
	std::ifstream file;
	BmpInforHearder Bmp;
	WORD check;
	INT tempInt;
	INT Size;
	INT tempImg;
	BYTE *rawdata, *reverse;
	BYTE tempByte;

	file.open(filename, std::ios::in | std::ios::binary);
	if (file.fail())
	{
		std::cout << "file open error in ReadBmp function" << std::endl;
		exit(1);
	}

	//check 'BM' type
	file.read((char*)&check, 2);
	if (check != 19778)
	{
		std::cerr << "error : image format is not a bmp" << std::endl;
		exit(1);
	}
	/* // alter
	if ((file.get() != 'B') || (file.get() != 'M'))
	{
	cerr << "errer : image format is not a bmp" << endl;
	exit(1);
	}*/

	file.read((char*)&tempInt, 4);					// file size
	file.read((char*)&tempInt, 4);					// reserved1
	file.read((char*)&tempInt, 4);					// reserved2
	file.read((char*)&tempInt, 4);					// offset

	//width & height
	file.read((char*)&Bmp.Width, 4);
	file.read((char*)&Bmp.Height, 4);
	*col = Bmp.Width;
	*row = Bmp.Height;

	file.read((char*)&tempInt, 2);					// colorplane is always 1

	//bit per pixel
	file.read((char*)&Bmp.BitCount, 2);

	file.read((char*)&tempInt, 4);					// compresstion flag
	file.read((char*)&tempInt, 4);					// raw data size
	file.read((char*)&tempInt, 4);					// width resolution
	file.read((char*)&tempInt, 4);					// height resolution

	//color number
	file.read((char*)&Bmp.NumOfColor, 4);
	Bmp.NumOfColor = Bmp.NumOfColor;

	file.read((char*)&tempInt, 4); // important color

	Size = Bmp.Width * Bmp.Height * 3;
	rawdata = new BYTE[Size];
	reverse = new BYTE[Size];

	if (Bmp.NumOfColor != 0)
	{
		std::cout << "not 24 bit" << std::endl;
		exit(1);
	}
	else
	{
		for (i = 0; i < Size * 3; i += 3)
		{
			if (((Bmp.Width % 4) != 0) && ((Bmp.Height % 3) == 0) && (i != 0))
			{
				for (j = 0; j < (4 - (Bmp.Width * 3) % 4); ++j)
				{
					file.read((char*)&tempByte, 1);
				}
			}
			tempImg = ((Bmp.Height - ((i / 3) / Bmp.Width) - 1)*Bmp.Width + (i / 3) % Bmp.Width) * 3;

			file.read((char*)(rawdata + tempImg + 2), 1);
			file.read((char*)(rawdata + tempImg + 1), 1);
			file.read((char*)(rawdata + tempImg), 1);
		}
	}
	file.close();

	CopyMemory(reverse, rawdata, Size);
	//std::memcpy(reverse, rawdata, Size);

	for (i = 0; i < Bmp.Height; ++i)
	{
		for (j = 0; j < Bmp.Width; ++j)
		{
			rawdata[i*Bmp.Width + j] = reverse[3 * (i*Bmp.Width + j)];
			rawdata[i*Bmp.Width + j + Bmp.Width] = reverse[3 * (i*Bmp.Width + j) + 1];
			rawdata[i*Bmp.Width + j + Bmp.Width * 2] = reverse[3 * (i*Bmp.Width + j) + 2];
		}
	}

	delete[] reverse;
	return rawdata;
}

VOID WritePpm(INT row, INT col, BYTE* img, LPCSTR  filename)
{

}
BYTE* ReadPpm(INT *row, INT *col, LPCSTR  filename)
{
	std::ifstream file;
	register INT i;
	INT size;
	BYTE *buffer, *rgbp = NULL, *rgbPtr;
	return rgbp;

}



template <class T>
DOUBLE gPSNR(T* origin, T* target, CONST INT length, CONST INT max)
{
	register INT i;
	DOUBLE PSNR = 0.0;
	DOUBLE MSE = 0.0;
	DOUBLE SUM = 0.0;

	for (i = 0; i < length; ++i)
	{
		SUM += (origin[i] - target[i]) * (origin[i] - target[i]);
	}

	MSE = SUM / length;

	PSNR = 20.0 * log10(max / sqrt(MSE));

	return PSNR;
}

template <class T>
DOUBLE gPSNR(T* origin, T* target, CONST INT height, CONST INT width, CONST INT max, CONST INT boundary)
{
	register INT i, j;
	DOUBLE PSNR = 0.0;
	DOUBLE MSE = 0.0;
	DOUBLE SUM = 0.0;
	LONG LENGTH = 0;

	for (i = boundary; i < height - boundary; ++i)
	{
		for (j = boundary; j < width - boundary; ++j)
		{
			SUM += (origin[i*width + j] - target[i*width + j])*(origin[i*width + j] - target[i*width + j]);
			LENGTH++;
		}
	}

	MSE = SUM / LENGTH;

	PSNR = 20.0 * log10(max / sqrt(MSE));

	return PSNR;
}
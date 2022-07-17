#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Defines a block of 512 bytes that stores the information of a part of the JPEG
#define BLOCK_SIZE 512

typedef uint8_t BYTE;

short is_jpeg(FILE *memoryCard, BYTE *JPEG_BLOCK);
short recover_jpeg(FILE *memoryCard);

int main(int argc, char *argv[])
{
    // Checks if the user used the program correctly
    if(argc != 2)
    {
        printf("Usage: ./recover IMAGE\n");
        return 1;
    }

    // Opens a filestream to access the "memory card" (in this case, is the card.raw file)
    FILE *memoryCard = fopen(argv[1], "r");
    if(memoryCard == NULL)
    {
        printf("Could not open the file\n");
        return 1;
    }

    short status = recover_jpeg(memoryCard);
    
    // Guarantees that the program quits execution if can't recover the images
    if(status != 1)
    {
        printf("Could not recover the photos. Aborting the program...\n");
        fclose(memoryCard);
        return 1;
    }
    
    fclose(memoryCard);

    return 0;
}

// Checks if a readen block is the beginning of a JPEG
short is_jpeg(FILE *memoryCard, BYTE *JPEG_BLOCK)
{
    fread(JPEG_BLOCK, BLOCK_SIZE, 1, memoryCard); // Reads into a JPEG_BLOCK one BLOCK_SIZE from the memoryCard file

    // Checks if I've read a block of a JPEG image
    if(JPEG_BLOCK[0] == 0xff && JPEG_BLOCK[1] == 0xd8 && JPEG_BLOCK[2] == 0xff)
    {
        for(int fourthJpegByte = 0xe0; fourthJpegByte <= 0xef; fourthJpegByte++)
        {
            if(JPEG_BLOCK[3] == fourthJpegByte)
            {
                return 1; // True
            }
        }
    }

    if(feof(memoryCard))
        return 2; // Reached the end of file

    return 0; // False
}

// Recovers the JPEG images from the car.raw file
short recover_jpeg(FILE *memoryCard)
{
    // Creates the buffer to store the readen data
    BYTE *JPEG_BLOCK = malloc(BLOCK_SIZE * sizeof(BYTE));
    if(JPEG_BLOCK == NULL)
    {
        return 3; // Run out of memory
    }

    // Guarantees that I'm reading a JPEG file
    short isJpeg = 0;
    while(isJpeg == 0)
    {
        isJpeg = is_jpeg(memoryCard, JPEG_BLOCK);
    }

    int i = 0;
    // Creates a string of size "000.jpg"
    char filename[sizeof("000.jpg")];

    // Loops over the card.raw file to find the JPEGs
    while(!feof(memoryCard))
    {
        // Prints a formated string into the filename string
        sprintf(filename, "%.3d.jpg", i);

        FILE *recoveredJpeg = fopen(filename, "w");
        if(recoveredJpeg == NULL)
        {
            return 2; // Couldn't open the file
        }

        // Writes the buffer content into a .jpg file
        isJpeg = 0;
        while(isJpeg == 0)
        {
            fwrite(JPEG_BLOCK, BLOCK_SIZE, 1, recoveredJpeg);

            // Ensures that I'm not writing another JPEG into the same file
            isJpeg = is_jpeg(memoryCard, JPEG_BLOCK);
            if(isJpeg == 1)
            {
                fclose(recoveredJpeg);
            }
        }

        i++;
    }

    free(JPEG_BLOCK);
    return 1; // Successfully recovered the JPEG
}
#include "data_io.h"
/*
// This takes the command line input file arguments
int main(int n_file, char* filename[])
{
	printf("\nNumber of files: %i", n_file - 1); //print out the input number of files

	// Check the number of command-line arguments
	if (n_file < 2)
	{
		//print out the correct program usage for the user if program is used incorrectly
		printf("\nUsage: program.exe <input_file1> [<input_file2> <input_file3> ...]");
		return 1;
	}

	// Process each input file
	for (int i_file = 1; i_file < n_file; i_file++)
	{
		printf("\nReading %s", filename[i_file]);
		vector<asciiPTC> pts = read_asciiTxt(filename[i_file]);

		printf("\nWriting %s", add_extension(filename[i_file], "_cpy", true).c_str());
		Write_asciiTxt(add_extension(filename[i_file], "_cpy", true), pts);

		printf("\nDone!\n");
	}
	printf("\nAll Done!");
	getchar();
	return 0;
}

HMODULE laszipDLL = LoadLibrary("laszip3.dll");
    if (!laszipDLL) {
        fprintf(stderr, "Failed to load laszip3.dll\n");
        return 1;
    }

*/


int main(int n_file, char* filename[])
{   
    printf("\nNumber of files: %i", n_file - 1);

    if (n_file < 2)
    {
        printf("\nUsage: program.exe <input_file1> [<input_file2> <input_file3> ...]");
        return 1;
    }

    // Process each input file
    for (int i_file = 1; i_file < n_file; i_file++)
    {
        string current_file = filename[i_file];

        // Check if the file has a .txt extension
        if (get_extension(current_file) == ".txt")
        {
            // Perform ASCII txt functions
            printf("\nReading %s", current_file.c_str());
            vector<asciiPTC> pts = read_asciiTxt(current_file);

            printf("\nWriting %s", add_extension(current_file, "_cpy", true).c_str());
            Write_asciiTxt(add_extension(current_file, "_cpy", true), pts);

            printf("\nDone with ASCII txt operations\n");
        }
        // Check if the file has a .las OR .laz extension
        else if (get_extension(current_file) == ".laz" || get_extension(current_file) == ".las")
        {
            // Perform LAS functions
            printf("\nReading %s", current_file.c_str());
            vector<LasPoint> lasPoints = read_LasFile(current_file);

            printf("\n\nComputing Statistics of %s", current_file.c_str());
            PointStatistics PtcStats = computeStatistics(lasPoints);
            printStatistics(PtcStats);

            printf("Hi Kenneth we are finally here");
            getchar();

            if (!lasPoints.empty())
            {
                printf("\nWriting %s", add_extension(current_file, "_cpy", true).c_str());
                write_LasFile(add_extension(current_file, "_cpy", true), lasPoints);

                printf("\nDone with LAS operations!\n");
            }
            else
            {
                printf("\nFailed to read all points in %s\n", current_file.c_str());
            }
        }
        // Check if the file has a .obj extension
        else if (get_extension(current_file) == ".obj") 
        {
            // Perform OBJ functions
            printf("\nReading %s", current_file.c_str());
            pair<vector<objVtx>, vector<objFct>>  objPoints = read_obj(current_file);
            printf("\nWriting %s", current_file.c_str());
            write_obj(add_extension(current_file, "_cpy", true), objPoints);

            printf("\nDone with OBJ operations!\n");
        }
        else
        {
            printf("\nUnsupported file extension for %s\n", current_file.c_str());
        }
    }

    printf("\nAll Done!");

    getchar();
    return 0;
}


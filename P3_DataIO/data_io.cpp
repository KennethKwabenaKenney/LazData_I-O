#include "data_io.h"

// Function to read an ascii text file, with the file as input
// and return the point cloud elements
vector<asciiPTC> read_asciiTxt(string file_in)
{
    vector<asciiPTC> pts;   //create a variable pts of the asciiPTC structure
    FILE* fp = fopen(file_in.c_str(), "rt");

    char buff[256];
    while (fgets(buff, 256, fp))
    {
        if (buff[0] != '#' && buff[0] != 'X' && buff[0] != 'x' && buff[0] != '\n')
        {
            //char c_temp; //use this to store characters separating elements
            asciiPTC pt_temp;   // temporary file to store the file elements
            // Read and store the elements in the txt file
            sscanf(buff, "%lf %lf %lf %d %d %d %d",
                &pt_temp.x,
                &pt_temp.y,
                &pt_temp.z,
                &pt_temp.r,
                &pt_temp.g,
                &pt_temp.b,
                &pt_temp.intensity);
            pts.push_back(pt_temp); //Store the read elements back into the ptc variable of the asciiPTC structure
            //testing            
            //printf("\n%lf %lf %lf %d %d %d %d", pt_temp.x, pt_temp.y, pt_temp.z, pt_temp.r, pt_temp.g, pt_temp.b, pt_temp.intensity);
            //getchar();
        }
    }

    fclose(fp);
    return pts;
}

// Function to write the ascii text file
void Write_asciiTxt(string file_o, vector<asciiPTC>& pts)
{
    FILE* fp = fopen(file_o.c_str(), "wt");

    fprintf(fp, "X, Y, Z, R, G, B, Intensity\n");

    for (int i = 0; i < pts.size(); i++)
    {
        fprintf(fp, "%lf, %lf, %lf, %i, %i, %i, %i\n", pts[i].x, pts[i].y, pts[i].z, pts[i].r, pts[i].g, pts[i].b, pts[i].intensity);
    }

    fclose(fp);
}

string add_extension(string path, string new_ext, bool behind_name_or_ext)
{
    if (behind_name_or_ext)
    {
        string ext = get_extension(path);

        return (remove_extension(path) + new_ext + ext);
    }
    else
    {
        return path + new_ext;
    }
}

string remove_extension(string path)
{
    string ext = get_extension(path);
    return path.substr(0, path.length() - ext.length());
}

string get_extension(string path)
{
    std::filesystem::path p = path;
    return p.extension().string();
}

// Function to read the las file. Input is the path of the LAS file. Returns a vector of LasPoint Structure of the points read
vector<LasPoint> read_LasFile(string file_in)
{
    vector<LasPoint> points;

    // Create a laszip reader
    laszip_POINTER las_reader;
    laszip_create(&las_reader);
  
    // Open the LAS file for reading
    laszip_BOOL is_compressed = 0;
    if (laszip_open_reader(las_reader, file_in.c_str(), &is_compressed))
    {
        fprintf(stderr, "Failed to open LAS file for reading: %s\n", file_in.c_str());
        laszip_close_reader(las_reader);
        return points;
    }

    // get a pointer to the header of the reader that was just populated
    laszip_header* header_read;
    if (laszip_get_header_pointer(las_reader, &header_read))
    {
        fprintf(stderr, "DLL ERROR: getting header pointer from laszip reader\n");
        laszip_close_reader(las_reader);
        return points;
    }

    // how many points are in the file 
    laszip_I64 npoints = (header_read->number_of_point_records ? header_read->number_of_point_records : header_read->extended_number_of_point_records);
    fprintf(stderr, "file '%s' \n contains %I64d points\n", file_in.c_str(), npoints);

    // get a pointer to the point of the reader will be read
    laszip_point* point_read;
    if (laszip_get_point_pointer(las_reader, &point_read))
    {
        fprintf(stderr, "DLL ERROR: getting point pointer from laszip reader\n");
        laszip_close_reader(las_reader);
        return points;
    }

    // read the points
    //laszip_I64 p_count = 0;

    for (laszip_I64 p_count = 0; p_count < npoints; ++p_count)
    {
        // read a point
       if (laszip_read_point(las_reader))
       {
           fprintf(stderr, "DLL ERROR: reading point %I64d\n", p_count);
           laszip_close_reader(las_reader);
           return points;
       }

        // copy the points
        LasPoint point;
        point.x = (double) (point_read->X * header_read->x_scale_factor) + header_read->x_offset;
        point.y = (double) (point_read->Y * header_read->y_scale_factor) + header_read->y_offset;
        point.z = (double) (point_read->Z * header_read->z_scale_factor) + header_read->z_offset;
        point.intensity = point_read->intensity;
        point.r = point_read->rgb[0];
        point.g = point_read->rgb[1];
        point.b = point_read->rgb[2];
   
        points.push_back(point);

        //printf("\n%lf %lf %lf %u %u %u %u", point.x, point.y, point.z, point.intensity, point.r, point.g, point.b);
        //getchar();
    }

    // Close the las fine    
    laszip_close_reader(las_reader);
    laszip_destroy(las_reader);
    
    return points;
}


void write_LasFile(string file_o, const vector<LasPoint>& lasPoints)
{
    // Create a laszip writer
    laszip_POINTER las_writer;
    if (laszip_create(&las_writer))
    {
        fprintf(stderr, "DLL ERROR: creating laszip writer\n");
        laszip_close_writer(las_writer);
    }

    // get a pointer to the header of the writer to populate it
    laszip_header* header;
    if (laszip_get_header_pointer(las_writer, &header))
    {
        fprintf(stderr, "DLL ERROR: getting header pointer from laszip writer\n");
        laszip_close_writer(las_writer);
    }

    //Compute point cloud stats to fill Min & Max_X Y Z
    PointStatistics PtcStats = computeStatistics(lasPoints);

    // populate the header                  **From ASPRS LAS Specification**
    header->file_source_ID = 4711;      // 0 - 65535, 0 = no assigned ID
    header->global_encoding = 0x0001; // time stamps are in adjusted standard GPS time
    header->version_major = 1;      // LAS version 1.4 (1 major)
    header->version_minor = 4;      // LAS version 1.4 (4 minor)
    strncpy(header->system_identifier, "OTHER", 32);    // Some other operation up to 32 characters
    header->file_creation_day = 33;     // Day of the year at time of file creation
    header->file_creation_year = 2024;  // Year at time of file creation
    header->header_size = 375;                 // size in bytes, 375 for LAS 1.4
    header->offset_to_point_data = 375;        // at least 375 bytes for LAS 1.4
    header->number_of_variable_length_records = 0;
    header->point_data_format = 2;        //Point Data Record Format 2 (addition of 3 color channels RGB)
    header->point_data_record_length = 26;      // Point Format 2 has Minimum PDRF Size of 26 bytes. Extra Bytes are user-specific “Extra Bytes” & can optionally be described with an Extra Bytes VLR
    header->number_of_point_records = 0;       // must be zero for point type 6 or higher
    for (int i = 0; i < 5; i++)
    {
        header->number_of_points_by_return[i] = 0;  // must be zero for point type 6 or higher (3-5 for Point Format 2)
    }
    header->max_x = PtcStats.maxX;      // actual unscaled XYZ extents of the LAS point data
    header->min_x = PtcStats.minX;
    header->max_y = PtcStats.maxY;
    header->min_y = PtcStats.minY;
    header->max_z = PtcStats.maxZ;
    header->min_z = PtcStats.minZ;
    header->extended_number_of_point_records = PtcStats.numPoints;  // Total point records
    for (int i = 0; i < 14; i++)
    {
        header->extended_number_of_points_by_return[i] = 0;
    }
    
    // use the bounding box and the scale factor to create a "good" offset
    if (laszip_auto_offset(las_writer))
    {
        fprintf(stderr, "DLL ERROR: during automatic offset creation\n");
        laszip_close_writer(las_writer);
    }

    // enable the compatibility mode
    laszip_BOOL request = 1;
    if (laszip_request_compatibility_mode(las_writer, request))
    {
        fprintf(stderr, "DLL ERROR: enabling laszip LAS 1.4 compatibility mode\n");
        laszip_close_writer(las_writer);
    }

    // open the writer
    laszip_BOOL compress = ((file_o.c_str(), ".laz") != 0);
    if (laszip_open_writer(las_writer, file_o.c_str(), compress))
    {
        fprintf(stderr, "DLL ERROR: opening laszip writer for '%s'\n", file_o.c_str());
        laszip_close_writer(las_writer);
    }

    // get a pointer to the point of the writer that we will populate and write
    laszip_point* point;
    if (laszip_get_point_pointer(las_writer, &point))
    {
        fprintf(stderr, "DLL ERROR: getting point pointer from laszip writer\n");
        laszip_close_writer(las_writer);
    }

    
    // write the points cloud
    int npoints = PtcStats.numPoints;
    laszip_F64 coordinates[3];
    laszip_I64 p_count = 0;
    for (laszip_I64 pc = 0; pc < npoints; ++pc)
    {
        // get the current point from the vector
        const LasPoint& currentPoint = lasPoints[pc];

        // populate the first point
        coordinates[0] = currentPoint.x;
        coordinates[1] = currentPoint.y;
        coordinates[2] = currentPoint.z;

        if (laszip_set_coordinates(las_writer, coordinates))
        {
            fprintf(stderr, "DLL ERROR: setting coordinates for point %I64d\n", npoints);
            laszip_close_writer(las_writer);
        }

        point->intensity = currentPoint.intensity;
        point->rgb[0] = currentPoint.r;
        point->rgb[1] = currentPoint.g;
        point->rgb[2] = currentPoint.b;

        if (laszip_write_point(las_writer))
        {
            fprintf(stderr, "DLL ERROR: writing point %I64d\n", npoints);
            laszip_close_writer(las_writer);
        }
        ++p_count; // Counter
    }

    fprintf(stderr, "\nsuccessfully written %I64d points\n", p_count);

    // Close the LAS file
    laszip_close_writer(las_writer);
    laszip_destroy(las_writer);
}


// Template function to compute statistics for a vector of points
template <typename PointType>
PointStatistics computeStatistics(const vector<PointType>& points) {
    PointStatistics stats;

    // Check if the vector is empty
    if (points.empty()) {
        printf("Vector is empty. Point Cloud statistics can't be computed.\n");
        stats.numPoints = 0;
        return stats;
    }

    // Compute the number of points
    stats.numPoints = points.size();

    // Initialize min and max values with the first point in the vector
    stats.minX = stats.maxX = points[0].x;
    stats.minY = stats.maxY = points[0].y;
    stats.minZ = stats.maxZ = points[0].z;

    // Iterate through the vector to find min and max values for each dimension
    for (const PointType& point : points) {
        stats.minX = min(stats.minX, point.x);
        stats.maxX = max(stats.maxX, point.x);
        stats.minY = min(stats.minY, point.y);
        stats.maxY = max(stats.maxY, point.y);
        stats.minZ = min(stats.minZ, point.z);
        stats.maxZ = max(stats.maxZ, point.z);
    }

    return stats;
}

// Function to print PointStatistics in a table format
void printStatistics(const PointStatistics& stats) {
    // Set precision for floating-point output
    std::cout << std::fixed << std::setprecision(6);

    // Print table header
    std::cout << std::setw(20) << std::left << "\n\nStatistic"
        << std::setw(15) << std::right << "Value" << std::endl;
    std::cout << std::setfill('-') << std::setw(35) << "-" << std::setfill(' ') << std::endl;

    // Print statistics
    std::cout << std::setw(20) << std::left << "Number of Points"
        << std::setw(15) << std::right << stats.numPoints << std::endl;
    std::cout << std::setw(20) << std::left << "X values - Min"
        << std::setw(15) << std::right << stats.minX << std::endl;
    std::cout << std::setw(20) << std::left << "X values - Max"
        << std::setw(15) << std::right << stats.maxX << std::endl;
    std::cout << std::setw(20) << std::left << "Y values - Min"
        << std::setw(15) << std::right << stats.minY << std::endl;
    std::cout << std::setw(20) << std::left << "Y values - Max"
        << std::setw(15) << std::right << stats.maxY << std::endl;
    std::cout << std::setw(20) << std::left << "Z values - Min"
        << std::setw(15) << std::right << stats.minZ << std::endl;
    std::cout << std::setw(20) << std::left << "Z values - Max"
        << std::setw(15) << std::right << stats.maxZ << std::endl;
}

// Function to read the Obj Point Cloud file type
pair<vector<objVtx>, vector<objFct>> read_obj(string file_in)
{
    vector<objVtx> vertices;   // create a vector of objVtx to store vertices
    vector<objFct> facets;     // create a vector of objFct to store facets
    FILE* fp = fopen(file_in.c_str(), "rt");

    char buff[256];
    while (fgets(buff, 256, fp))
    {
        if (buff[0] != '#' && buff[0] != 'X' && buff[0] != 'x' && buff[0] != '\n' && buff[0] != 'g' && buff[0] == 'v')
        {
            char c_temp;
            objVtx vertex_temp;   // temporary structure to store the file elements
            sscanf(buff, "%c %lf %lf %lf",
                &c_temp,
                &vertex_temp.Vx,
                &vertex_temp.Vy,
                &vertex_temp.Vz);

            vertices.push_back(vertex_temp);
        }
        if (buff[0] != '#' && buff[0] != 'X' && buff[0] != 'x' && buff[0] != '\n' && buff[0] != 'g' && buff[0] == 'f') {
            char c_temp;
            objFct facet_temp;   // temporary structure to store the file elements
            sscanf(buff, "%c %d %d %d",
                &c_temp,
                &facet_temp.Fx,
                &facet_temp.Fy,
                &facet_temp.Fz);

            facets.push_back(facet_temp);
        }
    }

    fclose(fp);

    return make_pair(vertices, facets);
}

// Function to write obj file
void write_obj(string file_o, const pair<vector<objVtx>, vector<objFct>>& VtxFct_Obj) {

    FILE* fp = fopen(file_o.c_str(), "wt");

    int numVtx = VtxFct_Obj.first.size();
    fprintf(fp, "#\n");
    fprintf(fp, "# %d vertices\n", numVtx);
    fprintf(fp, "#\n");

    // Write vertices
    for (const auto& vertex : VtxFct_Obj.first) {
        fprintf(fp, "v %lf %lf %lf\n", vertex.Vx, vertex.Vy, vertex.Vz);
    }

    int numFct = VtxFct_Obj.second.size();
    fprintf(fp, "#\n");
    fprintf(fp, "# %d facets\n", numFct);
    fprintf(fp, "#\n");

    // Write facets
    for (const auto& facet : VtxFct_Obj.second) {
        fprintf(fp, "f %d %d %d\n", facet.Fx, facet.Fy, facet.Fz);
    }
    fprintf(fp, "#\n");
    fprintf(fp, "# End of file.\n");

    fclose(fp);
}

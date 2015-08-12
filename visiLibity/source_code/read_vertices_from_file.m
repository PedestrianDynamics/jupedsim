%
%This function reads lists of vertices from the file filename 
%and returns them in a Matlab cell array (environment).  filename 
%is the name of a file in the working directory which lists 
%vertices in the human-readable VisiLibity Environment or Guards
%file formats as used in example.environment and 
%example.guards.
%

function environment = read_vertices_from_file(filename)


%%%--Read Lists of Vertices in the Plane from File--%%%
%Note:  the numbers in the file must always have decimal points.
environment = cell(1);  %Initialize cell array to hold polygons.
fid = fopen(filename,'r');
polygon_counter = 0;    %Keep track of how many holes so far.
point_counter = 0;      %Keep track of how many pts so far per hole.
line = fgetl(fid);
previous_line = line;
point_temp = zeros(1, 2);


while 1 %~feof(fid) %~strcmp( line(1:5), '//EOF' )
    
    
    if line(1) == '/'
        previous_line = line;
        line = fgetl(fid);
        continue;
    end
   
    
    if previous_line(1) == '/'
        polygon_counter = polygon_counter + 1;
        point_counter = 1;
    else
        point_counter = point_counter + 1;
    end

    
    point_temp = sscanf(line,'%f')';
    
    
    if feof(fid)
        environment{polygon_counter}(point_counter,:) = point_temp;
        fclose(fid);
        return;
    end


    environment{polygon_counter}(point_counter,:) = point_temp;
    previous_line = line;
    line = fgetl(fid);

    
end

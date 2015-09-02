%
%=========VisiLibity Demonstration Script=========
%
%This script uses the the MEX-files generated from 
%shortest_path.cpp and in_environment.cpp.  Follow the 
%instructions in the respective .cpp files to create
%these MEX-files before running this script. A graphical
%representation of the supplied environment file 
%example2.environment is dislplayed and the user can
%then select 2 points (must be in the environment) with
%the mouse and the shortest path between those points
%will be computed and plotted over the environment.
%


%Clear the desk
clear all; close all; clc;
format long;


%Robustness constant
epsilon = 0.0000000001;


%Snap distance (distance within which an observer location will be snapped to the
%boundary before the visibility polygon is computed)
snap_distance = 0.05;


%Read environment geometry from file
environment = read_vertices_from_file('./example2.environment');


%Calculate a good plot window (bounding box) based on outer polygon of environment
environment_min_x = min(environment{1}(:,1));
environment_max_x = max(environment{1}(:,1));
environment_min_y = min(environment{1}(:,2));
environment_max_y = max(environment{1}(:,2));
X_MIN = environment_min_x-0.1*(environment_max_x-environment_min_x);
X_MAX = environment_max_x+0.1*(environment_max_x-environment_min_x);
Y_MIN = environment_min_y-0.1*(environment_max_y-environment_min_y);
Y_MAX = environment_max_y+0.1*(environment_max_y-environment_min_y);


%Clear plot and form window with desired properties
clf; set(gcf,'position',[200 500 700 600]); hold on;
axis equal; axis off; axis([X_MIN X_MAX Y_MIN Y_MAX]);


%Plot environment.
patch( environment{1}(:,1) , environment{1}(:,2) , 0.1*ones(1,length(environment{1}(:,1)) ) , ...
       'w','linewidth',1.5);
for i = 2 : size(environment,2)
    patch( environment{i}(:,1) , environment{i}(:,2) , 0.1*ones(1,length(environment{i}(:,1)) ) , ...
           'k' , 'EdgeColor' , [0 0 0] , 'FaceColor' , [0.8 0.8 0.8] , 'linewidth' , 1.5 );
end


%Select test points with mouse and plot resulting shortest path
while 0 < 1
    
    %Acquire start and finish points
    [start_x start_y] = ginput(1);
    [finish_x finish_y] = ginput(1);
  
    %Make sure the selected points are in the environment
    if ~in_environment( [start_x start_y] , environment , epsilon ) ... 
        || ~in_environment( [finish_x finish_y] , environment , epsilon )
        display('Selected points must be in the environment!');
        break;
    end
    
    %Clear plot from previous test and reform window with desired properties
    clf; set(gcf,'position',[200 500 700 600]); hold on; 
    axis equal; axis off; axis([X_MIN X_MAX Y_MIN Y_MAX]);
    
    %Plot environment
    patch( environment{1}(:,1) , environment{1}(:,2) , 0.1*ones(1,length(environment{1}(:,1)) ), ...
           'w','linewidth',1.5);
    for i = 2 : size(environment,2)
        patch( environment{i}(:,1), environment{i}(:,2), 0.1*ones(1,length(environment{i}(:,1)) ), ...
               'k' , 'EdgeColor' , [0 0 0] , 'FaceColor' , [0.8 0.8 0.8] , 'linewidth' , 1.5 );
    end
    
    %Plot start and finish points.
    plot3( start_x , start_y , 0.3 , ...
        'o' , 'Markersize' , 9 , 'MarkerEdgeColor' , 'y' , 'MarkerFaceColor' , 'k' );
    plot3( finish_x , finish_y , 0.3 , ...
        'o' , 'Markersize' , 9 , 'MarkerEdgeColor' , 'y' , 'MarkerFaceColor' , 'k' );
    
    
    %Compute and plot shortest path
    my_path{1} = shortest_path( [start_x start_y] , [finish_x finish_y] , ...
                                environment , epsilon , snap_distance );
    plot3( my_path{1}(:,1) , my_path{1}(:,2) , 0.2*ones( size(my_path{1},1) , 1 ) , ...
           'b-' , 'Markersize' , 12 , 'LineWidth' , 1.6 , 'color' , [0.4 0.4 0.9] );
    
end

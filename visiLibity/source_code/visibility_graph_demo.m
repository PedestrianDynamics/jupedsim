%
%=========VisiLibity Demonstration Script=========
%
%This script uses the MEX-files generated from 
%visibility_graph.cpp.  Follow the instructions in 
%visibility_graph.cpp to create the MEX-file before 
%running this script. A graphical representation of 
%the supplied environment file example1.environment 
%is dislplayed together with the guards and their 
%visibility graph (green edges).
%


%Clear the desk
clear all; close all; clc;
format long;


%Robustness constant
epsilon = 0.000000001;


%Read environment and guards geometry from files
environment = read_vertices_from_file('./example1.environment');
guards = read_vertices_from_file('./example1.guards'); guards = guards{1};


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


%Plot Environment
patch( environment{1}(:,1) , environment{1}(:,2) , 0.1*ones(1,length(environment{1}(:,1)) ) , ...
       'w' , 'linewidth' , 1.5 );
for i = 2 : size(environment,2)
    patch( environment{i}(:,1) , environment{i}(:,2) , 0.1*ones(1,length(environment{i}(:,1)) ) , ...
           'k' , 'EdgeColor' , [0 0 0] , 'FaceColor' , [0.8 0.8 0.8] , 'linewidth' , 1.5 );
end


%Plot Guards
plot3(guards(:,1), guards(:,2), 0.4*ones(1,length(guards(:,2))), ...
    'o','Markersize',9,'MarkerEdgeColor','y','MarkerFaceColor','k');


%Compute and Plot visibility Graph
visibility_adjacency_matrix = visibility_graph(guards, environment, epsilon);
for i = 1 : size( visibility_adjacency_matrix , 1 )
    for j = 1 : size( visibility_adjacency_matrix , 2 )
        if( visibility_adjacency_matrix(i,j) == 1 )
            plot3( [guards(i,1) guards(j,1)], [guards(i,2) guards(j,2)], 0.3*[1 1], ...
                'g', 'LineWidth', 0.5 , 'LineStyle' , '-' );
        end
    end
end

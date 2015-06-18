
DistributeInSubRoom(sr, N, allpos, &pid,dist,building);
sr 	 -> subroom
N  	 -> no of ped in subroom
pid 	 -> pedestrian id
dist 	 -> room
building -> building

void PedDistributor :: DistributeInSubRoom_MitchellAlg(int nAgents , std::vector<Point>& positions) {
  
    AgentsParameters* agents_para=para->GetGroupParameters();
    
    std::srand(time(0));
    int index = rand() % positions.size();
    std::vector<std::unique_ptr<Point>> pos;
    
    for (int i = 0; i < nAgents; ++i) {
        Pedestrian* ped = new Pedestrian();
        // PedIndex
        ped->SetID(*pid);
        ped->SetAge(para->GetAge());
        ped->SetGender(para->GetGender());
        ped->SetHeight(para->GetHeight());
        ped->SetFinalDestination(para->GetGoalId());
        ped->SetGroup(para->GetGroupId());
        ped->SetRouter(building->GetRoutingEngine()->GetRouter(para->GetRouterId()));
        
        JEllipse E = JEllipse();
        E.SetAv(agents_para->GetAtau());
        E.SetAmin(agents_para->GetAmin());
        E.SetBmax(agents_para->GetBmax());
        E.SetBmin(agents_para->GetBmin());
        ped->SetEllipse(E);
        ped->SetTau(agents_para->GetTau());
        ped->SetV0Norm(agents_para->GetV0(),
                  agents_para->GetV0DownStairs(),
                  agents_para->GetV0UpStairs());
        
        
        if (i > 0) {
            std::set<int> candidates;
            while (candidates.size() < 10)
                candidates.insert(rand() % positions.size());
            
            double dist = 0; 
            double longest = 0;
            for (auto itr :: candidates) {
                double shortest = 0;
                for (unsigned int j = 0; j < pos.size(); ++j)
                    dist = (positions[itr] - pos[j]).norm();
                    if (dist == 0 || dist < shortest)
                        shortest = dist;
                
                if (shortest > longest) {
                    longest = shortest;
                    index = itr;
                }
            }
        }
        
        pos.push_back(positions[index]);

        ped->SetPos(positions[index],true); //true for the initial position
        ped->SetBuilding(building);
        positions.erase(positions.begin() + index);
        ped->SetRoomID(para->GetRoomId(),"");
        ped->SetSubRoomID(r->GetSubRoomID());
        ped->SetPatienceTime(para->GetPatience());
        ped->SetPremovementTime(para->GetPremovementTime());
        ped->SetRiskTolerance(para->GetRiskTolerance());
        const Point& start_pos = para->GetStartPosition();


        if((std::isnan(start_pos._x)==0 ) && (std::isnan(start_pos._y)==0 ) ) {
            if(r->IsInSubRoom(start_pos)==false){
                Log->Write("ERROR: \t cannot distribute pedestrian %d in Room %d at fixed position %s",
                                    *pid, para->GetRoomId(), start_pos.toString().c_str());
                Log->Write("ERROR: \t Make sure that the position is inside the geometry and belongs to the specified room / subroom");
                exit(EXIT_FAILURE);
            }

            ped->SetPos(start_pos,true); //true for the initial position
            Log->Write("INFO: \t fixed position for ped %d in Room %d %s",
                    *pid, para->GetRoomId(), start_pos.toString().c_str());
        }

        //r->AddPedestrian(ped);
        building->AddPedestrian(ped);
        (*pid)++;
    }
}







#include "DiscoveringMetaStructure.h"
AdjList *dataAdj;
AdjListId *dataType; 
LinkTypeHashMap typeMapR;
map<int, string> CatName;
map<int, string> TypeName;
vector<int> srcData, dstData;

int MAX_METAGRAPH_NUM = 10;
int EACH_METAGRAPH_NUM = 3;
int MAX_METAGRAPH_HOP = 10;
int ENTITYNUM = 0;
int TRAINNUM=8; 	
int LINKTYPE = 0;
int LINKTYPEHalf = 0;


void loadLinkType(string &typeLocation)
{
	ifstream inType(typeLocation, ios::in);
	if (!inType.is_open())
	{
		cout << "Link_Type file cann't open " << endl;
		return;	
	}
	string line, linkType, preType, postType;
	int id;
	while (getline(inType, line))
	{							    
		istringstream in(line);
		in >> id;
		in >> linkType >> preType >> postType;
		Type tempType;
		tempType.linkType = linkType;
		tempType.preType = preType;
		tempType.postType = postType;
		typeMapR.insert(make_pair(id, tempType));
		LINKTYPE++;
	}
	LINKTYPEHalf = LINKTYPE;
	LINKTYPE = LINKTYPE * 2;
	inType.close();
	cout << "LinkTypeNum=" << LINKTYPEHalf << "\t   ";
}

void loadAdj(string adjLocation, int adjLen, int skipType)
{
	dataAdj = new AdjList();
	dataAdj->resize(adjLen);
	ifstream inAdj(adjLocation);
	if (!inAdj.is_open())
	{
		cout << "Coef file cann't open " << endl;
		return;
	}
	string line;
	int src, dst, type;
	float value, rvalue;
	ENTITYNUM = 0;
	while (getline(inAdj, line, '\n'))
	{
		istringstream in(line);
		in >> src;
		if (src > ENTITYNUM)
		{
			ENTITYNUM = src;
		}

		while (true)
		{
			if (!(in >> dst))
				break;
			in >> type;
			in >> value;
			in >> rvalue;

			if (skipType >= 0)
			{
				if (type == skipType || type == -skipType)
					continue;
			}
			if (dst > ENTITYNUM)
				ENTITYNUM = dst;
			Node temp;
			temp.id = dst;
			temp.type = type;
			temp.value = value;
			temp.rvalue = rvalue;

			dataAdj->at(src).push_back(temp);
		}
	}
	ENTITYNUM++;

	cout << " EntityNum=" << ENTITYNUM << "\t";
}

void loadCat(string typeLocation, string taxStr, string Catstr)
{
	ifstream intax(typeLocation);
	if (!intax.is_open())
	{
		cout << "Total_Type file cann't open " << endl;
		return;
	}
	dataType = new AdjListId();
	dataType->resize(ENTITYNUM);
	string myline;
	int id, baseid;
	while (getline(intax, myline))
	{
		istringstream in(myline);
		in >> baseid;
		while (in >> id)
		{
			dataType->at(baseid).push_back(id);
		}
	}
	intax.close();

	ifstream ifname(taxStr);
	if (!ifname.is_open())
	{
		cout << "Tax_Id file cann't open " << endl;
		return;
	}
	string inname, temp;
	int inid, myid;
	while (getline(ifname, myline, '\n'))
	{
		istringstream in(myline);
		in >> inid >> myid;
		while (in >> temp)
		{
			inname += " " + temp;
		}
		CatName[inid] = inname.substr(1);
		inname = "";
	}
	ifname.close();

	ifstream iftype(Catstr);
	if (!iftype.is_open())
	{
		cout << "Vertice_type file cann't open " << endl;
		return;
	}
	while (getline(iftype, myline))
	{
		istringstream in(myline);
		in >> inid >> inname;
		TypeName[inid] = inname;
	}
	iftype.close();
}

void loadTrainData(string dataPosLocation, string dataNegLocation)
{
	ifstream inPosData(dataPosLocation);
	ifstream inNegData(dataNegLocation);
	if (!inPosData.is_open()||!inNegData.is_open())
	{
		cout << "Train Data file cann't open " << endl;
		 return;
	}
	string myline;
	int src, dst,i=0;
	while (i<TRAINNUM)
	{
		if (i < TRAINNUM / 2)
		{
			inPosData >> src >> dst;
		}
		else
		{
			inNegData >> src >> dst;
		}
		srcData.push_back(src);
		dstData.push_back(dst);
		i++;
	}

	inPosData.close();
	inNegData.close();
	cout << "TrainExample_PairNum=" << TRAINNUM << endl << endl;
} 

void SetExperimentVariable()
{
	string str;

	cout << " Setting the number of Meta-Structure: ";
	cin >> str;
	MAX_METAGRAPH_NUM = atoi(str.c_str());

	cout << " Setting max_hop of each Meta-Structure: ";
	cin >> str;
	MAX_METAGRAPH_HOP = atoi(str.c_str());

	cout << " Setting max_num branch node of each Meta-Structure:";
	cin >> str;
	EACH_METAGRAPH_NUM = atoi(str.c_str());
	
	cout << "Setting train_num examples pairs:";
	cin >> TRAINNUM;
	cout << endl;

	getchar();
}

int dblp()
{
	cout << endl;
	cout << "       \t#################   " << "Discovering Meta-Structure" << "    ##################" << endl;
	cout << endl;

	//SetExperimentVariable();

	cout << "       \t\t##########   " << "Start Loading Data" << "    ##########" << endl << endl;

	string adjLocation = "F:\\Meta-structure Learning\\data\\dblp_four_area\\dblpCoef2.txt";
	int skipType = -1;
	int adjLen = 15649;
	loadAdj(adjLocation, adjLen, skipType);

	string linkTypeLocation = "F:\\Meta-structure Learning\\data\\dblp_four_area\\dblpType.txt";
	loadLinkType(linkTypeLocation);  // loading the link 

	string typeLocation = "F:\\Meta-structure Learning\\data\\dblp_four_area\\dblpTotalType.txt";
	string catLocation = "F:\\Meta-structure Learning\\data\\dblp_four_area\\dblpTaxID.txt";
	string verticeTypeLocation = "F:\\Meta-structure Learning\\data\\dblp_four_area\\dblpVerticeType.txt";
	loadCat(typeLocation, catLocation, verticeTypeLocation);

	string trainPosDataLocation = "F:\\Meta-structure Learning\\data\\dblp_four_area\\posTrainD.txt";
	string trainNegDataLocation = "F:\\Meta-structure Learning\\data\\dblp_four_area\\negTrainD.txt";
	loadTrainData(trainPosDataLocation, trainNegDataLocation);

	cout << "       \t\t##########   " << "End Loading Data" << "    ##########" << endl << endl;

	string temp(MAP_CODE_LEN, 'F');
	LinkTypeNode mapPathCode, mapStructureCode;
	mapPathCode.mark = temp;
	mapPathCode.firstUse = temp;
	mapStructureCode.mark = temp;
	mapStructureCode.firstUse = temp;

	cout << " ======>>Start Discovering  Potential Meta-Structure:" << endl;
	//getchar();

	testTopkMulti(dataAdj, dataType, &mapPathCode, &mapStructureCode, ENTITYNUM, srcData, dstData, MAX_METAGRAPH_NUM, EACH_METAGRAPH_NUM, MAX_METAGRAPH_HOP);

	return 1;
}
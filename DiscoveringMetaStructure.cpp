#include "DiscoveringMetaStructure.h"
#include "PreProcess.H"
#include "ToolBox.h"
#include "header.h"
#define ALPHA 1
#define BETA 0.6
;
hash_map<int, int> srcMap;
unordered_map<int, float> finalScore;
SpecialNodeTypeHashMap specialNodeType;
									   
float residual = 100.0f;
int itertime = 0;
int queSize;							
int typeCount;

unordered_map<int, vecNode*>  discoverNewBranch(vecNode *now, int currentType,LinkTypeNode *&mapPathCode)  //寻找所有可能的新的分支
{
	vector<Node>::const_iterator iter;
	storeMap::iterator niter;
	unordered_map<int, vecNode*> vecStore;
	for (niter = now->store->begin(); niter != now->store->end(); niter++)
	{
		int comid = niter->first;
		int id = comid / 1000;
		int sourceid = comid - id * 1000;

		float score = niter->second.score;
		bool pos = niter->second.pos;

		for (iter = dataAdj->at(id).begin(); iter != dataAdj->at(id).end(); iter++)
		{
			float nextvalue = score*pow(iter->value, ALPHA) * BETA;

			if (nextvalue < ZERO)
			{
				continue;
			}
			int nextid = iter->id;
			if (niter->second.store.find(nextid) != niter->second.store.end())
			{
				continue;
			}

			int nexttype = getPathType(currentType, iter->type + LINKTYPEHalf,now->pathORStruc,mapPathCode);
			unordered_map<int, vecNode*>::iterator hashfind = vecStore.find(nexttype);
			if (hashfind != vecStore.end())
			{
				storeMap::iterator nodeiter;
				nodeiter = hashfind->second->store->find(nextid * 1000 + sourceid);
				storeMap::iterator enditer = hashfind->second->store->end();
				if (hashfind->second->correct.at(sourceid) < nextvalue&&sourceid < TRAINNUM)
				{
					hashfind->second->correct[sourceid] = nextvalue;
				}

				if (nodeiter != enditer)
				{
					nodeiter->second.score += nextvalue;
					nodeiter->second.store.insert(nextid);
				}
				else
				{
					hashNode tempNode;
					tempNode.score = nextvalue;
					tempNode.store = niter->second.store;
					tempNode.store.insert(nextid);
					tempNode.pos = pos;
					hashfind->second->store->insert(make_pair(nextid * 1000 + sourceid, tempNode));
				}
			}
			else
			{
				storeMap *tempVec = new storeMap();
				hashNode tempNode;
				
				tempNode.score = nextvalue;
				tempNode.store = niter->second.store;
				tempNode.store.insert(nextid);
				tempNode.pos = pos;
				tempVec->insert(make_pair(nextid * 1000 + sourceid, tempNode));
				vecNode *invecNode = new vecNode(nexttype,false,now->nStrucNum,now->nHop+1,now->IfStruc,tempVec,0);
				
				if (invecNode->correct[sourceid] < nextvalue && sourceid < TRAINNUM)
				{
					invecNode->correct[sourceid] = nextvalue;
				}
				
				vecStore[nexttype] = invecNode;
			}
		}
	}
	return vecStore;
}

unordered_map<int, vecNode*> discoverSingleBranch(vecNode *now,int singleType)  ////只建立某一个类型的分支
{
	vector<Node>::const_iterator iter;
	storeMap::iterator niter;
	unordered_map<int, vecNode*> vecStore;
	
	for (niter = now->store->begin(); niter != now->store->end(); niter++)
	{
		int comid = niter->first;
		int id = comid / 1000;
		int sourceid = comid - id * 1000;

		float score = niter->second.score;
		bool pos = niter->second.pos;

		for (iter = dataAdj->at(id).begin(); iter != dataAdj->at(id).end(); iter++)
		{
			if (iter->type == singleType) //只建立某一个类型的分支
			{
				float nextvalue = score*pow(iter->value, ALPHA) * BETA;

				int nextid = iter->id;
				if (niter->second.store.find(nextid) != niter->second.store.end())
				{
					continue;
				}
				unordered_map<int, vecNode*>::iterator hashfind = vecStore.find(0);
				if (hashfind != vecStore.end())
				{
					storeMap::iterator nodeiter;
					nodeiter = hashfind->second->store->find(nextid * 1000 + sourceid);
					storeMap::iterator enditer = hashfind->second->store->end();
					if (nodeiter != enditer)
					{
						nodeiter->second.score += nextvalue;
						nodeiter->second.store.insert(nextid);
					}
					else
					{
						hashNode tempNode;
						tempNode.score = nextvalue;
						tempNode.store = niter->second.store;
						tempNode.store.insert(nextid);
						tempNode.pos = pos;
						hashfind->second->store->insert(make_pair(nextid * 1000 + sourceid, tempNode));
					}
				}
				else
				{
					storeMap *tempVec = new storeMap();
					//tempVec->reserve(20000);
					hashNode tempNode;
					tempNode.score = nextvalue;
					tempNode.store = niter->second.store;
					tempNode.store.insert(nextid);
					tempNode.pos = pos;
					tempVec->insert(make_pair(nextid * 1000 + sourceid, tempNode));
					vecNode *invecNode = new vecNode(0,false,now->nStrucNum,0,false,tempVec,0);
					vecStore[0] = invecNode;
				}
			}
		}
	}
	return vecStore;
}

void buildMetaStructureBranch(list<vecNode*> &que,vector<int> vecNodeType,vector<int> set_intersection,int nTypeNum,LinkTypeNode *&mapPathCode, LinkTypeNode *&mapStructureCode,myVector &myResidual, int currenttype, int nBranchNum, int nHop)
{
	int nodeType;
	nodeType = findLinkType(mapPathCode, vecNodeType[0]) + LINKTYPEHalf;
	if (nodeType >= 5)
		nodeType = nodeType - 1;
	if (mapStructureCode->firstUse[currenttype*LINKTYPE + nodeType] != 'T')				
	{
		mapStructureCode->id[currenttype*LINKTYPE + nodeType] = ++typeCount;
		mapStructureCode->mark[currenttype*LINKTYPE + nodeType] = 'T';
		mapStructureCode->firstUse[currenttype*LINKTYPE + nodeType] = 'T';
	}
	currenttype = mapStructureCode->id[currenttype*LINKTYPE + nodeType];
	for (int i = 1; i < nTypeNum - 1; i++)
	{
		nodeType = findLinkType(mapPathCode, vecNodeType[i]) + LINKTYPEHalf;
		if (nodeType >= 5)
			nodeType = nodeType - 1;
		if (mapStructureCode->firstUse[currenttype*LINKTYPE + nodeType] != 'T')
		{											   
			mapStructureCode->id[currenttype*LINKTYPE + nodeType] = ++typeCount;
			mapStructureCode->firstUse[currenttype*LINKTYPE + nodeType] = 'T';
		}
		currenttype = mapStructureCode->id[currenttype*LINKTYPE + nodeType];
	}

	nodeType = findLinkType(mapPathCode, vecNodeType[nTypeNum - 1]) + LINKTYPEHalf;
	if (nodeType >= 5)
		nodeType = nodeType - 1;
	mapStructureCode->id[currenttype*LINKTYPE + nodeType] = ++typeCount;

	for (int i = 0; i < nTypeNum; i++)
	{
		getPathType(typeCount, -findLinkType(mapPathCode, vecNodeType[i]) + LINKTYPEHalf, false, mapStructureCode);
	}
	storeMap *srcmap = new storeMap();
	for (int i = 0; i < set_intersection.size(); i++)
	{
		hashNode inNode;
		inNode.score = 1.0f;
		inNode.pos = true;
		unordered_set<int> rout;
		int id = set_intersection[i];

		rout.insert(id - id / 1000 * 1000);
		inNode.store = rout;
		srcmap->insert(make_pair(id, inNode));  //建立起树节点中的每一项											//TruthHash.insert(dstData[i] * 1000 + i);   //存储正例
	}
	vecNode *firstNode = new vecNode(typeCount, true, nBranchNum, nHop,true, srcmap,0);
	insertIntoList(que, firstNode,myResidual);
}

void generateMetaStructure(list<vecNode*> &que,vector<vecNode*> listBranchNode, LinkTypeNode *&mapPathCode,LinkTypeNode *&mapStructureCode ,myVector &myResidaul,int currenttype, int nBranchNum,int nHop)
{
	set<int> first, second;
	storeMap::iterator temp;
	vecNode *branchNode;
	vector<int> set_intersection;
	vector<int>vecNodeType;
	if(!listBranchNode.empty())
	{  
		branchNode = listBranchNode[0];
		for (temp =branchNode ->store->begin() ; temp != branchNode->store->end(); temp++)
		{
			int id = temp->first;
			first.insert(temp->first - (id / 1000) * 1000);
		}
	}
	else
	{
		cout << "The number of branch is ZERO";
		return ;
	}

	for(int i=0;i<listBranchNode.size();i++)
	{		
		branchNode = listBranchNode[i];
		vecNodeType.push_back(branchNode->type);
		for (temp = branchNode->store->begin(); temp != branchNode->store->end(); temp++)
		{
			int id = temp->first;
			second.insert(temp->first - (id / 1000) * 1000);
		}
		set_intersection = existSameElement(first, second);
		if (set_intersection.size() > 0)
		{
			first.clear();
			first.insert(set_intersection.begin(),set_intersection.end());
		}
		else
		{   
			cout << "First branch does not meet the condition" << endl;
			return ;
		}
	}

	unordered_map<int, vecNode*>firstStoreNode, secondStoreNode;
	int linkType= findLinkType(mapPathCode, vecNodeType[0]);
	firstStoreNode = discoverSingleBranch(listBranchNode[0], -linkType);
	first.clear();
	for (temp = firstStoreNode.begin()->second->store->begin(); temp != firstStoreNode.begin()->second->store->end(); temp++)
	{
		first.insert(temp->first);
	}
	delete(firstStoreNode.begin()->second->store);
	delete(firstStoreNode.begin()->second);

	for (int i = 1; i < listBranchNode.size(); i++)
	{
		linkType = findLinkType(mapPathCode, vecNodeType[i]);
		secondStoreNode = discoverSingleBranch(listBranchNode[i],-linkType);
		second.clear();				    
		for (temp = secondStoreNode.begin()->second->store->begin(); temp != secondStoreNode.begin()->second->store->end(); temp++)
		{
			second.insert(temp->first);
		}
		set_intersection= existSameElement(first, second);
		first.clear();
		first.insert(set_intersection.begin(), set_intersection.end());
		delete(secondStoreNode.begin()->second->store);
		delete(secondStoreNode.begin()->second);
		if (set_intersection.size() <= 0)
		{
			cout << "Second branch does not meet the condition" << endl;
			return;
		}
	}

	buildMetaStructureBranch(que, vecNodeType, set_intersection, vecNodeType.size(), mapPathCode, mapStructureCode,myResidaul, currenttype, nBranchNum, nHop);
}

bool compareVecNode(vecNode* first, vecNode* second)
{
	return (first->priorityScore) > (second->priorityScore);
	//return (first->g*1.0f ) > (second->g*1.0f);
}

void updateQueRank(list<vecNode *>&que, myVector &myResidual)
{
	list<vecNode*>::iterator iter;
	for (iter = que.begin(); iter != que.end(); iter++)
	{
		float res = 0;
		float secres = 0;
		int count = 0;
		for (int i = 0; i < TRAINNUM; i++)  //注意这里设定的分子只取样本的一半
		{
			if (i < TRAINNUM / 2)
			{
				res += (*iter)->correct[i] * myResidual(i);
			}
			secres += pow((*iter)->correct[i], 2);
		}
		(*iter)->priorityScore = res / secres;
	}
	que.sort(compareVecNode);
}

float checkExist(hash_map<int, myVector> tempStore, myVector &myResidual, int &type, myVector &res)
{
	hash_map<int, myVector>::const_iterator iter;
	float maxscore = -1;
	int maxtype;
	for (iter = tempStore.begin(); iter != tempStore.end(); iter++)
	{
		float score = 0;
		float base = 0;
		int count = 0;
		for (int i = 0; i < TRAINNUM; i++)
		{
			if (i < TRAINNUM / 2)
			{
				score += myResidual(i)*iter->second(i);
			}
			base += pow(iter->second(i), 2);
		}
		score = score / sqrt(base);
		if (score > maxscore)
		{
			maxscore = score;
			maxtype = iter->first;
		}
	}
	if (maxscore > 0.0000001f)
	{
		type = maxtype;
		res = tempStore[type];
	}
	return maxscore;

}

int dijTopkMultiCountM(AdjList *dataAdj, list<vecNode *>&que,hash_map<int, myVector> &tempStore,myVector &myResidual,unordered_set<int> dstVec,LongHashSet TruthHash, LinkTypeNode *&mapPathCode, LinkTypeNode *&mapStructureCode, vector<myVector> &myAdd,int &iterTime, int eachMetaStrucNum, int nMaxHopNum)
{
	
	updateQueRank(que, myResidual);
	float threshold = -1.0f;

	int storeType;
	myVector tempRes = myVector::Zero(TRAINNUM);
	threshold = checkExist(tempStore, myResidual, storeType, tempRes);

	if (que.front()->priorityScore < threshold)
	{
		cout << "  The " << iterTime << "-th: ";
		printMap(mapPathCode, mapStructureCode, false, storeType);  //此处默认为false
		myAdd[0] = tempRes;
		tempStore.erase(storeType);
		return 0;
	}
	
	while (!que.empty())
	{   
		//cout << que.front()->priorityScore << "     " << threshold << endl;
		if (que.front()->priorityScore < threshold)
		{
			cout << "  The " << iterTime << "-th: ";
			printMap(mapPathCode, mapStructureCode, que.front()->pathORStruc, storeType);  //此处默认为false
			myAdd[0] = tempRes;
			tempStore.erase(storeType);
			return 0;
		}

		unordered_map<int, vecNode*> vecStore;
		vecNode *now = que.front();
		que.pop_front();
		int currenttype = now->type;

		myVector exactOne(TRAINNUM);
		exactOne = myVector::Zero(TRAINNUM);

		if (checkExistAimNode(now, dstVec,TruthHash,exactOne,myResidual,tempStore,storeType,tempRes,threshold))  //判断是否发现目标对象，只要存在至少一个就输出该meta-structure
		{
			delete(now->store);
			delete(now);
			continue ; 
		}

		if (now->nHop < nMaxHopNum)
		{
			vecStore = discoverNewBranch(now, currenttype, mapPathCode);
		}
		else
		{
			delete(now->store);
			delete(now);
			continue;
		}

		if (vecStore.size() == 0)
		{
			delete(now->store);
			delete(now);

			continue;
		}
		else if (vecStore.size() == 1)
		{
			insertIntoList(que, vecStore.begin()->second,myResidual);
		}
		else
		{
			unordered_map<int, vecNode*>::iterator hashiter;
			for (hashiter = vecStore.begin(); hashiter != vecStore.end(); hashiter++)
			{
				insertIntoList(que, hashiter->second,myResidual);
			}

			/*vector<vecNode*> listBranchNode;

			unordered_map<int, vecNode*>::iterator firstIter, secondIter;
			int count = 0;

			for (firstIter = vecStore.begin(); firstIter != vecStore.end(); firstIter++)
			{
			for (secondIter = firstIter, ++secondIter; secondIter != vecStore.end(); secondIter++)
			{
			if (firstIter->second->nStrucNum < eachMetaStrucNum && secondIter->second->nStrucNum < eachMetaStrucNum)
			{
			listBranchNode.push_back(firstIter->second);
			listBranchNode.push_back(secondIter->second);
			generateMetaStructure(que, listBranchNode, mapPathCode, mapStructureCode,myResidual,currenttype, firstIter->second->nStrucNum + 1, firstIter->second->nHop + 1);
			listBranchNode.clear();
			}
			}
			}*/

		}
		delete(now->store);
		delete(now);
	}
	return 1;
}

void testTopkMulti(AdjList *dataAdj, AdjListId *typeAdj, LinkTypeNode *mapPathCode,LinkTypeNode *mapStructureCode ,int entityNum, vector<int>srcData, vector<int>dstData,int iterNum,int eachMetaStrucNum,int nMaxHopNum)
{
	int num = 1;
	unordered_set<int> dstVec;
	LongHashSet TruthHash;
	storeMap *srcmap = new storeMap(); //typedef unordered_map<int, hashNode> storeMap;

	for (int i = 0; i < TRAINNUM; i++)  //将example pairs载入程序，这里含有正反样本对
	{
		hashNode inNode;
		inNode.score = 1.0f;
		inNode.pos = true;								 
		unordered_set<int> rout;
		rout.insert(i);
		inNode.store = rout;
		srcmap->insert(make_pair((srcData[i] * 1000 + i), inNode));  //建立起树节点中的每一项
		if(i<TRAINNUM/2)
		{ 
			TruthHash.insert(dstData[i] * 1000 + i);   //存储正例
		}
		dstVec.insert(dstData[i]);
		
	}
	   
	list<vecNode*> que;
	int srctype = 0;
	vecNode *firstNode = new vecNode(srctype,false,0,0,false,srcmap,0);
	que.push_back(firstNode);
	int returnStatus = 0;

	myVector myResidual(TRAINNUM), myWeight(60);
	myWeight = myVector::Zero(60);
	myResidual = myVector::Ones(TRAINNUM);

	for (int i = 0; i < TRAINNUM; i++)
	{
		if (i < TRAINNUM / 2)
		{
			myResidual(i) = 1;
		}
		else
		{
			myResidual(i) = 0;
		}
	}

	float fResidual = 100.0f;
	int iterTime = 0;
	int returnValue;
	hash_map<int, myVector> tempStore;
	myMatrix myvec = myMatrix::Zero(TRAINNUM, 60);
	

	while (fResidual>0.0001f && iterTime<iterNum)
	{
		float curCov, nextCov;
		int topk = 0;
		if (iterTime == 0)
			topk = 2;
		else
			topk = 1;
		vector<myVector> myAdd(topk);
		for (int i = 0; i < topk; i++)
			myAdd.at(i) = myVector::Zero(TRAINNUM);

		returnValue=dijTopkMultiCountM(dataAdj, que,tempStore ,myResidual,dstVec ,TruthHash, mapPathCode, mapStructureCode, myAdd,iterTime, eachMetaStrucNum, nMaxHopNum);
	   
		if (returnValue == 0)
		{
			if (topk == 1)
			{
				myvec.col(itertime + 1) = myAdd.at(0);
			}
			else
			{
				myvec.col(itertime) = myVector::Zero(TRAINNUM);
				myvec.col(itertime + 1) = myAdd.at(0);
			}
			float cnorm = myvec.col(itertime).norm();
			float nnorm = myvec.col(itertime + 1).norm();

			curCov = (myvec.col(itertime).transpose()*myResidual);
			nextCov = (myvec.col(itertime + 1).transpose()*myResidual);

			getDirection(myvec, curCov, nextCov, iterTime, myWeight, myResidual, TRAINNUM);
			itertime++;
			fResidual = 0;

			for (int i = 0; i < TRAINNUM; i++)
			{
				fResidual += abs(myResidual(i));
			}
		}		
		else
		{
			cout << "Meta-path Finished!" << endl;
		}
		iterTime++;
		cout << fResidual << endl;
	}
}

	

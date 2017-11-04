#include "header.h"
#include "DiscoveringMetaStructure.h"
#include "PreProcess.h"

int findLinkType(LinkTypeNode *&mapPathCode, int src)
{
	int type = src;
	int rtype;

	for (int i = src - 1; i >= 0; i--)
	{
		for (int j = 0; j < LINKTYPE; j++)
		{
			if (mapPathCode->id[i*LINKTYPE + j] == type)
			{
				type = i;
				if (j > 3)
				{
					return j - LINKTYPEHalf + 1;
				}
				else
				{
					return j - LINKTYPEHalf;
				}
			}
		}
	}
}

int getPathType(int prepath, int nodetype, bool pathOrStructure, LinkTypeNode *&mapPathCode)
{
	int pathid = 0;

	if (nodetype >= 5)
		nodetype = nodetype - 1;

	pathid = mapPathCode->id[prepath*LINKTYPE + nodetype];
	int result;

	if (pathid > 0)
	{

		result = pathid;
	}
	else
	{
		result = ++typeCount;
		mapPathCode->id[prepath*LINKTYPE + nodetype] = result;
		if (pathOrStructure)
		{
			mapPathCode->mark[prepath*LINKTYPE + nodetype] = 'T';
		}
	}
	return result;
}
	
float getDirection(myMatrix &all, float CurrentCov, float nextCov, int nextid, myVector &myweight, myVector &myresidual, int sizeLen)
{
	myMatrix Xa = all.block(0, 0, sizeLen, nextid + 1);  //取现有的已找到的向量
	myMatrix GA = Xa.transpose()*Xa;
	myMatrix ridge(nextid + 1, nextid + 1);
	ridge = myMatrix::Zero(nextid + 1, nextid + 1);
	double small = 0.00001f;
	for (int i = 0; i < nextid + 1; i++)
	{
		ridge(i, i) = small;
	}
	myMatrix invG = (GA + ridge).inverse();
	myVector SA = myVector::Ones(nextid + 1);
	double LA = 1 / sqrt(SA.transpose()*invG*SA);
	myVector wA = LA*invG*SA;
	myVector uA = Xa*wA;
	myVector a = all.block(0, 0, sizeLen, nextid + 1).transpose()*uA;
	double stepSize = (CurrentCov - nextCov)*1.0f / (LA - a(nextid)); //xxx


	if (stepSize <= 0 || (LA - a(nextid + 0)<0.000001f && LA - a(nextid + 0)>-0.000001f))
		stepSize = (CurrentCov + nextCov)*1.0f / (LA + a(nextid));
	myVector addweight = stepSize*wA;  //更新权值

	for (int i = 0; i < nextid + 1; i++)
	{
		myweight(i) += addweight(i);   //更新权值
	}
	myresidual = myresidual - all*myweight;  //更新残差
	return stepSize;
}

bool checkExistAimNode(vecNode *now,unordered_set<int> dstVec,LongHashSet TruthHash,myVector &exactOne,myVector &myResidual, hash_map<int, myVector> &tempStore, int &storeType, myVector &tempRes,float threshold)  //检查是否发现目标节点
{
	storeMap::iterator niter;
	for (niter = now->store->begin(); niter != now->store->end(); niter++)
	{
		int comid = niter->first;
		int id = comid / 1000;
		int sourceid = comid - id * 1000;
		float score = niter->second.score;
		bool pos = niter->second.pos;
	
		if (TruthHash.find(comid) != TruthHash.end())
		{
			exactOne[sourceid] = score;
		}

	}
	float resExactOne = 0.0f;
	float basenum = 0.0f;
	for (int i = 0; i<TRAINNUM; i++)
	{
		if (i < TRAINNUM / 2)
		{
			resExactOne += exactOne[i] * myResidual(i);
		}
		basenum += pow(exactOne[i], 2.0);
	}

	if (resExactOne > 0)
	{
		tempStore[now->type] = exactOne;
		if (resExactOne > threshold)
		{
			tempRes = exactOne;
			storeType = now->type;
		}
		return true;
	}
	return false;
}

vector<int> existSameElement(set<int> first, set<int> second)  //检验两个集合是否含有相同的元素
{
	vector<int> sameElement;
	auto iter = set_intersection(first.begin(), first.end(), second.begin(), second.end(), inserter(sameElement,sameElement.begin()));
	return sameElement;
}

void printMap(LinkTypeNode *&mapPathCode, LinkTypeNode *&mapStructureCode,bool pathORStruc,int src)  //输出meta-graph
{
	int type1 = src;
	int type2;
	int rtype;
	bool first = true;
	stack<string> outEdge;
	bool temp=false;
	nodeHashMap::const_iterator iter;
	if (pathORStruc)
	{
		for (int i = src - 1; i >= 0; i--)
		{
			for (int j = 0; j < LINKTYPE; j++)
			{
				if (mapStructureCode->id[i*LINKTYPE + j] == type1)
				{
					type1 = i;
					rtype = j - LINKTYPEHalf;
					if (rtype >= 0)
					{
						if (mapStructureCode->mark[i*LINKTYPE + j] == 'T')
						{
							outEdge.push("*" + typeMapR[rtype + 1].linkType + "*");
							//cout << "*" + typeMapR[rtype + 1].linkType + "*" << char(27);
							type1=src = i;
							temp = true;
							break;
						}
						else
						{
							outEdge.push("*" + typeMapR[rtype + 1].linkType + "*");
							// << "*" + typeMapR[rtype + 1].linkType + "*" << char(27);
						}
					}
					else
					{
						if (mapStructureCode->mark[i*LINKTYPE + j] == 'T')
						{
							outEdge.push("*-" + typeMapR[-rtype].linkType + "*");
							//cout << "*-" + typeMapR[-rtype].linkType + "*" << char(27);
							type1=src = i;
							temp = true;
							break;
						}
						else
						{
							outEdge.push("*-" + typeMapR[-rtype].linkType + "*");
							//cout << "*-" + typeMapR[-rtype].linkType + "*" << char(27);
						}
					}
					if (temp)
					{
						break;
					}
				}
			}
		}
	}

	temp = false;

	for (int i = src - 1; i >= 0; i--)
	{
		for (int j = 0; j < LINKTYPE; j++)
		{
			temp = false;
			if (mapPathCode->id[i*LINKTYPE + j] == type1)
			{
				type1 = i;
				rtype = j - LINKTYPEHalf;
				if (rtype >= 0)
				{
					if (mapPathCode->mark[i*LINKTYPE + j] == 'T')
					{
						outEdge.push(typeMapR[rtype + 1].linkType);
						//cout<<typeMapR[rtype + 1].linkType << char(27);
						type2 = i;
						for (int m = i - 1; m >= 0; m--)
						{
							for (int n = 0; n < LINKTYPE; n++)
							{
								if (mapStructureCode->id[m*LINKTYPE + n] == type2)
								{
									type2 = m;
									rtype = n - LINKTYPEHalf;
									if (rtype >= 0)
									{
										outEdge.push("*" + typeMapR[rtype + 1].linkType + "*");
										//cout << "*" + typeMapR[rtype + 1].linkType + "*" << char(27);
									}
									else
									{
										outEdge.push("*-" + typeMapR[-rtype].linkType + "*");
										//cout << "*-" + typeMapR[-rtype].linkType + "*" << char(27);
									}
									if (mapStructureCode->mark[m*LINKTYPE + n] == 'T'&&mapPathCode->mark[m*LINKTYPE + n] != 'T')
									{
										type1=src=i= m;
										temp = true;
										break;
									}
								}

							}
							if (temp)
							{
								break;
							}
						}
					}
					else
					{
						outEdge.push(typeMapR[rtype + 1].linkType);
						//cout << typeMapR[rtype + 1].linkType << char(27);
					}
				}
				else
				{
					if (mapPathCode->mark[i*LINKTYPE + j] == 'T')
					{
						outEdge.push("-" + typeMapR[-rtype].linkType);
						//cout<<"*-" + typeMapR[-rtype].linkType + "*"<<char(27);
						type2 = i;
						for (int m = i - 1; m >= 0; m--)
						{
							for (int n = 0; n < LINKTYPE; n++)
							{
								if (mapStructureCode->id[m*LINKTYPE + n] == type2)
								{
									type2 = m;
									rtype = n - LINKTYPEHalf;
									if (rtype >= 0)
									{
										outEdge.push("*" + typeMapR[rtype + 1].linkType + "*");
										//cout << "*" + typeMapR[rtype + 1].linkType + "*" << char(27);
									}
									else
									{
										outEdge.push("*-" + typeMapR[-rtype].linkType + "*");
										//cout << "*-" + typeMapR[-rtype].linkType + "*" << char(27);
									}
									if (mapStructureCode->mark[m*LINKTYPE + n] == 'T'&&mapPathCode->mark[m*LINKTYPE + n] != 'T')
									{
										type1=i =src= m;
										temp = true;
										break;
									}
								}

							}
							if (temp)
							{
								break;
							}
						}
					}
					else
					{
						outEdge.push("-" + typeMapR[-rtype].linkType);
						//cout << "-" + typeMapR[-rtype].linkType << char(27);
					}
				}
				if (temp)
					break;
				if (type1 == 0)
					break;
			}
		}
	}
	
	while (!outEdge.empty())
	{
		cout << outEdge.top();
		outEdge.pop();
		if (!outEdge.empty())
			cout << char(26);
	}
	cout << endl;
}

void insertIntoList(list<vecNode*> &que, vecNode *invec, myVector &residual)
{
	list<vecNode*>::iterator iter;
	float res = 0.0f;
	float tempres = 0.0f;
	int count = 0;

	for (int i = 0; i < TRAINNUM; i++)
	{
		res += invec->correct[i] * residual(i);
		tempres += pow(invec->correct[i], 2.0);
	}
	invec->priorityScore = res / sqrt(tempres);


	if (que.size() == 0)
	{
		que.push_front(invec);
	}
	else
	{	
		for (iter = que.begin(); iter != que.end(); iter++)
		{
			if (invec->priorityScore >= (*iter)->priorityScore)
			{
				que.insert(iter, invec);
				return;
			}
		}
		if (iter == que.end())
			que.insert(que.end(), invec);
	}
}

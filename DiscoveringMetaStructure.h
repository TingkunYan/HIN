#ifndef META_STRUCTURE_H
#define META_STRUCTURE_H
#include "header.h"

#define MAP_CODE_LEN 5000000

extern int TRAINNUM;

struct Node //邻接矩阵节点
{
	int id;
	float value;
	int type;
	float rvalue;
};

struct Type //连接边类型
{
	string linkType;
	string preType;
	string postType;
};

struct LinkTypeNode	  
{
	int *id = new int[MAP_CODE_LEN]();
	string mark;
	string firstUse;
};

struct hashNode
{
	float score;
	bool pos;
	unordered_set<int> store;
};

typedef unordered_map<int, hashNode> storeMap;

struct vecNode
{
	int type;
	bool pathORStruc;
	int nStrucNum;
	int nHop;
	bool IfStruc;
	vector<float> correct;
	float priorityScore;
	storeMap *store;  //typedef unordered_map<int, hashNode> storeMap;
	vecNode(int t, bool pOrS, int nB,int nL,bool bS, storeMap *instore, float fS) :type(t), pathORStruc(pOrS), nStrucNum(nB), nHop(nL),IfStruc(bS),store(instore),priorityScore(fS)
	{
		for (int i = 0; i < TRAINNUM; i++)
		{
			correct.push_back(0); 
		}
	}
	vecNode() {};
};

typedef hash_set<long long> LongHashSet;
typedef vector<vector<Node>> AdjList;
typedef vector<vector<int>> AdjListId;
typedef hash_map<int, Node> nodeHashMap;
typedef vector<nodeHashMap> AdjHash;
typedef hash_map<int, Type> LinkTypeHashMap;
typedef hash_map<int, vector<int>> SpecialNodeTypeHashMap;

extern int typeCount;

void testTopkMulti(AdjList *dataAdj, AdjListId *typeAdj, LinkTypeNode *mapPathCode, LinkTypeNode *mapStructureCode, int entityNum, vector<int>srcData, vector<int>dstData, int trainNum, int eachMetaStrucNum, int maxHopNum);

int dblp();
#endif // !META_STRUCTURE_H
#pragma once

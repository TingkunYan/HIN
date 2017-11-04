#ifndef TOOL_BOX_H
#define TOOL_BOX_H

int findLinkType(LinkTypeNode *&mapPathCode, int src);
int getPathType(int prepath, int nodetype, bool pathOrStructure,LinkTypeNode *&mapPathCode);
float getDirection(myMatrix &all, float CurrentCov, float nextCov, int nextid, myVector &myweight, myVector &myresidual, int sizeLen);
bool checkExistAimNode(vecNode *now,unordered_set<int> dstVec,LongHashSet TruthHash, myVector &exactOne, myVector &myResidual, hash_map<int, myVector> &tempStore, int &storeType,myVector &tempRes, float threshold);  //检查是否发现目标节点
vector<int> existSameElement(set<int> first, set<int> second);  //检验两个集合是否含有相同的元素
void printMap(LinkTypeNode *&mapPathCode, LinkTypeNode *&mapStructureCode, bool pathORStruc,int src);  //输出meta-
void insertIntoList(list<vecNode*> &que, vecNode *invec, myVector &residual)
#endif // !TOOL_BOX_H


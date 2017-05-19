/*
txt2scene
by mjt, 2014

Converts .txt map file to urho3d scene .xml file.

*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <string>
using namespace std;

class ModelInfo
{
public:
	char name[100];
	char ch;
	float xs, ys, zs;
};
ModelInfo modelInfo[1000];

void main(int argc, char **argv)
{
	if(argc<3)
	{
		printf("txt2scene by mjt, 2014\n");
		printf("Usage:\n %s  txtMapFileName  outputSceneFileName\n", argv[0]);
		return;
	}

	// open txt map file
	FILE *in=fopen(argv[1], "r");
	if(!in)
	{
		printf("Cannot open %s\n", argv[1]);
		return;
	}

#define MAX 1000
	char line[MAX];
	char mapName[MAX];
	int modelCounter=0;
	int width=0, height=0, blockWidth=0, blockHeight=0;
	float curX=0, curZ=0;
	string data=
		"<?xml version=\"1.0\"?>\n" \
		"<scene id=\"1\">\n" \
		"   <attribute name=\"Name\" value=\"\" />\n" \
		"   <attribute name=\"Time Scale\" value=\"1\" />\n" \
		"   <attribute name=\"Smoothing Constant\" value=\"50\" />\n" \
		"   <attribute name=\"Snap Threshold\" value=\"5\" />\n" \
/*		"   <attribute name=\"Elapsed Time\" value=\"7.3882\" />\n" \
		"   <attribute name=\"Next Replicated Node ID\" value=\"15\" />\n" \
		"   <attribute name=\"Next Replicated Component ID\" value=\"24\" />\n" \
		"   <attribute name=\"Next Local Node ID\" value=\"16777222\" />\n" \
		"   <attribute name=\"Next Local Component ID\" value=\"16777217\" />\n" \  
		"   <attribute name=\"Variables\" />\n" \
		"   <attribute name=\"Variable Names\" value=\"\" />\n" \  */
		"   <component type=\"Octree\" id=\"1\" />\n" \
		"   <component type=\"DebugRenderer\" id=\"2\" />\n";


	mapName[0]=0;
	int nodeID=1, compID=1;

	while(1)
	{
		if(fgets(line, MAX, in)==0)
			break;
		if(line[0]==0) 
			break;

		if(line[0]=='\n')
			continue;
		if(line[0]=='#') // comment
			continue;

		// map size
		if(memcmp(line, "mapSize:", 8)==0)
		{
			sscanf(line, "mapSize: %d %d", &width, &height);
			printf("mapSize: %d %d\n", width, height);

			continue;
		}

		if(memcmp(line, "blockSize:", 10)==0)
		{
			sscanf(line, "blockSize: %d %d", &blockWidth, &blockHeight);
			printf("blockSize: %d %d\n", blockWidth, blockHeight);

			continue;
		}

		// model name
		if(memcmp(line, "model:", 6)==0)
		{
			float xs=1, ys=1, zs=1;
			sscanf(line, "model: %c %s %f %f %f", 
				&modelInfo[modelCounter].ch, modelInfo[modelCounter].name, &xs, &ys, &zs);

			modelInfo[modelCounter].xs=xs;
			modelInfo[modelCounter].ys=ys;
			modelInfo[modelCounter].zs=zs;
			printf("model: %c = %s  (%8.2f, %8.2f, %8.2f)\n", 
				modelInfo[modelCounter].ch, modelInfo[modelCounter].name, xs, ys, zs);
		
			modelCounter++;

			continue;
		}

		// map name (ie floor, walls, furnitures)
		if(line[0]==':')
		{
			if(mapName[0]!=0)
				data+="   </node>\n";

			sscanf(line, ":%s\n", mapName);
			printf("name: %s\n", mapName);

			curX= -((float)width * (float)blockWidth)/2;
			curZ= -((float)height * (float)blockHeight)/2;

			char temp[100];
			sprintf(temp, "   <node id=\"%d\">\n", nodeID++);
			data+=temp;

			sprintf(temp, "   <attribute name=\"Name\" value=\"%s\" />\n", mapName);
			data+=temp;

			continue;
		}

		// parse map data
		for(int w=0; w<width; w++)
		{
			char *modelName="";
			float sx=1, sy=1, sz=1;
			for(int c=0; c<modelCounter; c++)
			{
				if(modelInfo[c].ch == line[w])
				{
					modelName = modelInfo[c].name;
					sx=modelInfo[c].xs;
					sy=modelInfo[c].ys;
					sz=modelInfo[c].zs;
					break;
				}
			}

			// dont add empty nodes
			if(memcmp(modelName, "empty", 5)==0)
			{
				curX += (float)blockWidth;
				continue;
			}

			char dta[10000];
			sprintf(dta, 
				"      <node id=\"%d\">\n" \
				"         <attribute name=\"Is Enabled\" value=\"true\" />\n" \
				"         <attribute name=\"Name\" value=\"%s\" />\n" \
				"         <attribute name=\"Position\" value=\"%8.2f %8.2f %8.2f\" />\n" \
				"         <attribute name=\"Rotation\" value=\"1 0 0 0\" />\n" \
				"         <attribute name=\"Scale\" value=\"%8.2f %8.2f %8.2f\" />\n" \
				"         <component type=\"StaticModel\" id=\"%d\">\n" \
				"            <attribute name=\"Model\" value=\"Model;Models/%s.mdl\" />\n" \
				"            <attribute name=\"Material\" value=\"Material;Materials/%s.xml;\" />\n" \
				"         </component>\n" \
				"      </node>\n",
					nodeID,
					modelName,
					curX, 0.f, curZ,
					sx, sy, sz,
					compID,
					modelName,
					modelName // material name
					);

			data = data + dta;

			nodeID++;
			compID++;

			curX += (float)blockWidth;
		}

		curX = -((float)width * (float)blockWidth)/2;
		curZ += (float)blockHeight;
	}

	fclose(in);

	data+= "   </node>\n";
	data+= "</scene>\n";

	// open output file
	FILE *out=fopen(argv[2], "w");
	if(!out)
	{
		printf("Cannot create %s\n", argv[1]);
		return;
	}
	fputs(data.c_str(), out);
	fclose(out);

}

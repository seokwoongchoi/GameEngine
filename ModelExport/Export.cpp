#include "stdafx.h"
#include "Export.h"
#include "Assimp/Loader.h"

void Export::Initialize()
{	
	//House();
	//Grass();
	//Police();
	//Bridge();
	//Douglas();
	//Paladin();
	//Car();
	//Tree();
	//Tank();
	//Tower();
	//Kachujin();
	//Samurai();
	//Eclipse();
	//Nyra();
	//Katanami();
	//Shogun();
	//Lamp();
	//Alex();
	//Katana();
	//M4();
	//SWAT();
	//Mask();
	//James();
}

void Export::House()
{
	
	Loader* loader = new Loader();
	loader->ReadFile(L"House/Verde Residence.obj");
	loader->ExportMaterial(L"VerdeResidence/VerdeResidence",false);//읽을때는 true로 바꿔서해야한다.
	loader->ExportMesh(L"VerdeResidence/VerdeResidence");
	SafeDelete(loader);
}

void Export::Grass()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Grass/free grass by adam127.fbx");
	loader->ExportMaterial(L"Grass/Grass",false);//읽을때는 true로 바꿔서해야한다.
	loader->ExportMesh(L"Grass/Grass");
	SafeDelete(loader);
}

void Export::Tank()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Tank/tank.fbx");
	loader->ExportMaterial(L"Tank/Tank",false);//읽을때는 true로 바꿔서해야한다.
	loader->ExportMesh(L"Tank/Tank");
	SafeDelete(loader);
	
}

void Export::Tower()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Tower/Tower.fbx");
	loader->ExportMaterial(L"Tower/Tower");
	loader->ExportMesh(L"Tower/Tower");
	SafeDelete(loader);
}

void Export::Kachujin()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Kachujin/Kachujin.fbx");
	loader->ExportMaterial(L"Kachujin/Kachujin",false);
	loader->ExportMesh(L"Kachujin/Kachujin");
	SafeDelete(loader);

	//loader = new Loader();
	//loader->ReadFile(L"Kachujin/Fight_Idle.fbx");
	//loader->ExportAnimClip(0, L"Kachujin/Fight_Idle");
	////vector<wstring>list;
	////loader->GetClipList(&list);
	//SafeDelete(loader);

	//loader = new Loader();
	//loader->ReadFile(L"Kachujin/Macarena_Dance.fbx");
	//loader->ExportAnimClip(0, L"Kachujin/Macarena_Dance");
	////vector<wstring>list;
	////loader->GetClipList(&list);
	//SafeDelete(loader);
	
}

void Export::Eclipse()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Eclipse/Eclipse.fbx");
	loader->ExportMaterial(L"Eclipse/Eclipse");
	loader->ExportMesh(L"Eclipse/Eclipse");
	SafeDelete(loader);
}

void Export::Samurai()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Samurai2/Samurai2.fbx");
	loader->ExportMaterial(L"Samurai2/Samurai2",false);
	loader->ExportMesh(L"Samurai2/Samurai2");
	SafeDelete(loader);

	loader = new Loader();
	loader->ReadFile(L"Samurai2/Standing Melee Attack 360 High.fbx");
	loader->ExportAnimClip(0, L"Samurai2/Standing Melee Attack 360 High");
	//vector<wstring>list;
	//loader->GetClipList(&list);
	SafeDelete(loader);

	//loader = new Loader();
	//loader->ReadFile(L"Samurai2/Draw_Sword_1.fbx");
	//loader->ExportAnimClip(1, L"Samurai2/Draw_Sword_1");
	////vector<wstring>list;
	////loader->GetClipList(&list);
	//SafeDelete(loader);
}

void Export::Nyra()
{
	Loader* loader = new Loader();
	
	loader->ReadFile(L"Nyra/Nyra_pose.fbx");
	//loader->ReadFileNyra(L"Nyra/Draw_Sword_1.fbx");
	loader->ExportMaterial(L"Nyra/Nyra");
	loader->ExportMesh(L"Nyra/Nyra");
	SafeDelete(loader);

	loader = new Loader();
	loader->ReadFile(L"Nyra/Draw_Sword_1.fbx");
	loader->ExportAnimClip(0, L"Nyra/Draw_Sword_1");
	/*ector<wstring>list;
	loader->GetClipList(&list);*/
	SafeDelete(loader);
	

	/*loader = new Loader();
	loader->ReadFile(L"Nyra/Run.fbx");
	loader->ExportAnimClip(0, L"Nyra/Run");*/
	//vector<wstring>list;
	//loader->GetClipList(&list);
	//SafeDelete(loader);
}

void Export::Katanami()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Katanami/Katanami.fbx");
	loader->ExportMaterial(L"Katanami/Katanami",false);
	loader->ExportMesh(L"Katanami/Katanami");
	SafeDelete(loader); 

	loader = new Loader();
	loader->ReadFile(L"Katanami/Fight_Idle.fbx");
	loader->ExportAnimClip(0, L"Katanami/Fight_Idle");
	/*ector<wstring>list;
	loader->GetClipList(&list);*/
	SafeDelete(loader);


}

void Export::Shogun()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Shogun/Character/FBX/Body.fbx");
	
	//loader->ExportMaterial(L"Shogun/Shogun");
	loader->ExportMesh(L"Shogun/Shogun_Body");
	SafeDelete(loader);

	loader = new Loader();
	loader->ReadFile(L"Shogun/Character/Animations/Walk_F.FBX");
	loader->ExportAnimClip(0, L"Shogun/Walk_F");
	/*ector<wstring>list;
	loader->GetClipList(&list);*/
	SafeDelete(loader);

	//loader = new Loader();
	//loader->ReadFile(L"Shogun/Character/Animations/Walk_F.FBX");
	//loader->ExportAnimClip(0, L"Shogun/Walk_F");
	///*ector<wstring>list;
	//loader->GetClipList(&list);*/
	//SafeDelete(loader);
}

void Export::Tree()
{
	//Loader* loader = new Loader();
	//loader->ReadFile(L"Tree/tree_topol.fbx");
	//loader->ExportMaterial(L"Tree/tree_topol");
	//loader->ExportMesh(L"Tree/tree_topol");
	//SafeDelete(loader);

	Loader* loader = new Loader();
	loader->ReadFile(L"Tree/bush-01.fbx");
	loader->ExportMaterial(L"Trees/bush");
	loader->ExportMesh(L"Trees/bush");
	SafeDelete(loader);


}

void Export::Car()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Car/Range rover.FBX");
	loader->ExportMaterial(L"Range rover/Range rover",false);
	loader->ExportMesh(L"Range rover/Range rover");
	SafeDelete(loader);
}

void Export::Paladin()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Paladin/paladin_prop_j_nordstrom.FBX");
	loader->ExportMaterial(L"Paladin/paladin_prop_j_nordstrom");
	loader->ExportMesh(L"Paladin/paladin_prop_j_nordstrom");
	SafeDelete(loader);
}

void Export::Douglas()
{
	
	/*Loader* loader = new Loader();
	loader->ReadFile(L"Douglas/Douglas.FBX");
	loader->ExportMaterial(L"Douglas/Douglas",false);
	loader->ExportMesh(L"Douglas/Douglas");
	SafeDelete(loader);*/
	
	Loader* loader = new Loader();
	loader->ReadFile(L"Douglas/Hit Reaction.fbx");
	loader->ExportAnimClip(0, L"Douglas/Hit Reaction");
	//vector<wstring>list;
	//loader->GetClipList(&list);
	SafeDelete(loader);

	loader = new Loader();
	loader->ReadFile(L"Douglas/LeftStrafe.fbx");
	loader->ExportAnimClip(0, L"Douglas/LeftStrafe");
	//vector<wstring>list;
	//loader->GetClipList(&list);
	SafeDelete(loader);

	loader = new Loader();
	loader->ReadFile(L"Douglas/RightStrafe.fbx");
	loader->ExportAnimClip(0, L"Douglas/RightStrafe");
	//vector<wstring>list;
	//loader->GetClipList(&list);
	SafeDelete(loader);


	loader = new Loader();
	loader->ReadFile(L"Douglas/Dying.fbx");
	loader->ExportAnimClip(0, L"Douglas/Dying");
	//vector<wstring>list;
	//loader->GetClipList(&list);
	SafeDelete(loader);
}

void Export::Bridge()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Bridge/Bridge.FBX");
	loader->ExportMaterial(L"Bridge/Bridge",false);
	loader->ExportMesh(L"Bridge/Bridge");
	SafeDelete(loader);
}

void Export::Police()
{
	/*Loader* loader = new Loader();
	loader->ReadFile(L"Police/Police.blend");
	loader->ExportMaterial(L"Police/Police");
	loader->ExportMesh(L"Police/Police");
	SafeDelete(loader);*/

	Loader* loader = new Loader();
	loader->ReadFile(L"Police/Ford Crown Victoria  LAPD.blend");
	loader->ExportMaterial(L"Police/LAPD");
	loader->ExportMesh(L"Police/LAPD");
	SafeDelete(loader);
}

void Export::Lamp()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Lamp/Gamelantern_updated1.fbx");
	loader->ExportMaterial(L"Lamp/Lamp",false);
	loader->ExportMesh(L"Lamp/Lamp");
	SafeDelete(loader);

	/*Loader* loader = new Loader();
	loader->ReadFile(L"Lamp/street_lamp.fbx");
	loader->ExportMaterial(L"StreetLamp/StreetLamp",false);
	loader->ExportMesh(L"StreetLamp/StreetLamp");
	SafeDelete(loader);*/
}

void Export::Alex()
{
	/*Loader* loader = new Loader();
	loader->ReadFile(L"Alex/Alex.fbx");
	loader->ExportMaterial(L"Alex/Alex",false);
	loader->ExportMesh(L"Alex/Alex");
	SafeDelete(loader);
*/
	
	Loader* loader = new Loader();
	loader->ReadFile(L"Alex/Hit Reaction.fbx");
	loader->ExportAnimClip(0, L"Alex/Hit Reaction");
	//vector<wstring>list;
	//loader->GetClipList(&list);
	SafeDelete(loader);

	loader = new Loader();
	loader->ReadFile(L"Alex/LeftStrafe.fbx");
	loader->ExportAnimClip(0, L"Alex/LeftStrafe");
	//vector<wstring>list;
	//loader->GetClipList(&list);
	SafeDelete(loader);

	loader = new Loader();
	loader->ReadFile(L"Alex/RightStrafe.fbx");
	loader->ExportAnimClip(0, L"Alex/RightStrafe");
	//vector<wstring>list;
	//loader->GetClipList(&list);
	SafeDelete(loader);

	
}

void Export::Katana()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"katana/katana.obj");
	loader->ExportMaterial(L"katana/katana");
	loader->ExportMesh(L"katana/katana");
	SafeDelete(loader);
}

void Export::M4()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"M4/M4.obj");
	loader->ExportMaterial(L"M4/M4",false);
	loader->ExportMesh(L"M4/M4");
	SafeDelete(loader);
}

void Export::SWAT()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Swat/swat.fbx");
	loader->ExportMaterial(L"Swat/Swat");
	loader->ExportMesh(L"Swat/Swat");
	SafeDelete(loader);


	// loader = new Loader();
	//loader->ReadFile(L"Swat/000_Rifle Idle.fbx");
	//loader->ExportAnimClip(0, L"Swat/000_Rifle Idle");
	////vector<wstring>list;
	////loader->GetClipList(&list);
	//SafeDelete(loader);

	// loader = new Loader();
	//loader->ReadFile(L"Swat/Firing Rifle.fbx");
	//loader->ExportAnimClip(0, L"Swat/Firing Rifle");
	////vector<wstring>list;
	////loader->GetClipList(&list);
	//SafeDelete(loader);
	// loader = new Loader();
	//loader->ReadFile(L"Swat/Dying.fbx");
	//loader->ExportAnimClip(0, L"Swat/Dying");
	////vector<wstring>list;
	////loader->GetClipList(&list);
	//SafeDelete(loader);
}

void Export::Mask()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"Mask/Mask.fbx");
	loader->ExportMaterial(L"Mask/Mask");
	loader->ExportMesh(L"Mask/Mask");
	SafeDelete(loader);
}

void Export::James()
{
	Loader* loader = new Loader();
	loader->ReadFile(L"James/James.fbx");
	loader->ExportMaterial(L"James/James");
	loader->ExportMesh(L"James/James");
	SafeDelete(loader);
}


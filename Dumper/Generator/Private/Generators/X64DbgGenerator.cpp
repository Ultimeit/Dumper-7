
#include <fstream>

#include "Generators/X64DbgGenerator.h"


void X64DbgGenerator::GenerateVTableName(nlohmann::json& jsonArray, UEObject DefaultObject)
{
	
	UEClass Class = DefaultObject.GetClass();
	UEClass Super = Class.GetSuper().Cast<UEClass>();

	if (Super && DefaultObject.GetVft() == Super.GetDefaultObject().GetVft())
		return;

	std::string Name = Class.GetCppName() + "_VFT";

	uint32 Offset = static_cast<uint32>(GetOffset(DefaultObject.GetVft()));

	nlohmann::json commentNode;
	commentNode["text"] = Name;
	commentNode["address"] = std::format("0x{:x}", Offset);
	commentNode["module"] = ModuleName;

	jsonArray.insert(jsonArray.end(), commentNode);
}

void X64DbgGenerator::GenerateClassFunctions(nlohmann::json& jsonArray, UEClass Class)
{
	static std::unordered_map<uint32, std::string> Funcs;

	for (UEFunction Func : Class.GetFunctions())
	{
		if (!Func.HasFlags(EFunctionFlags::Native))
			continue;

		std::string MangledName = Class.GetCppName() + "::exec" + Func.GetValidName();

		uint32 Offset = static_cast<uint32>(GetOffset(Func.GetExecFunction()));

		nlohmann::json commentNode;
		commentNode["text"] = MangledName;
		commentNode["address"] = std::format("0x{:x}", Offset);
		commentNode["module"] = ModuleName;

		jsonArray.insert(jsonArray.end(), commentNode);
	}
}

void X64DbgGenerator::WriteGlobal(nlohmann::json& jsonArray, const char* Name, int32 RVA)
{
	nlohmann::json lblNode;
	lblNode["module"] = ModuleName;
	lblNode["text"] = Name;
	lblNode["address"] = std::format("0x{:x}", RVA);
	jsonArray.insert(jsonArray.end(), lblNode);
}

void X64DbgGenerator::Generate()
{
	std::string IdaMappingFileName = (Settings::Generator::GameVersion + '-' + Settings::Generator::GameName + ".dd64");

	FileNameHelper::MakeValidFileName(IdaMappingFileName);

	/* Open the stream as binary data, else ofstream will add \r after numbers that can be interpreted as \n. */
	std::ofstream x64dbgDump(MainFolder / IdaMappingFileName, std::ios::binary);

	nlohmann::json comments = nlohmann::json::array({ });
	
	char buf[MAX_PATH];	// I'm too lazy to bother doing something better than this.
	DWORD sz = GetModuleFileNameA(NULL, buf, MAX_PATH);

	std::string pathToModule(buf, sz);
	ModuleName = pathToModule.substr(pathToModule.find_last_of('\\')+1);

	for (UEObject Obj : ObjectArray())
	{
		if (Obj.HasAnyFlags(EObjectFlags::ClassDefaultObject))
		{
			/* Gets the VTable offset from the default object and writes the ClassName + "_VFT" postfix to the file */
			GenerateVTableName(comments, Obj);
		}
		else if (Obj.IsA(EClassCastFlags::Class))
		{
			/* Iterates all of the functions of the class and them to the stream with an "exec" prefix in front of the function name */
			GenerateClassFunctions(comments, Obj.Cast<UEClass>());
		}
	}

	nlohmann::json labels = nlohmann::json::array({ });

	WriteGlobal(labels, "GObjects", Off::InSDK::ObjArray::GObjects);
	WriteGlobal(labels, "AppendNameToString", Off::InSDK::Name::AppendNameToString);
	WriteGlobal(labels, "GNames", Off::InSDK::NameArray::GNames);
	WriteGlobal(labels, "GWorld", Off::InSDK::World::GWorld);
	WriteGlobal(labels, "ProcessEvent", Off::InSDK::ProcessEvent::PEOffset);

	nlohmann::json j;
	j["comments"] = comments;
	j["labels"] = labels;

	x64dbgDump << j.dump(1);
}
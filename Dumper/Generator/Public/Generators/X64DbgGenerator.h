#pragma once

#include <iostream>
#include <string>

#include "Unreal/ObjectArray.h"
#include "PredefinedMembers.h"
#include "../Utils/Json/json.hpp"


class X64DbgGenerator
{
public:
    static inline PredefinedMemberLookupMapType PredefinedMembers;

    static inline std::string MainFolderName = "x64dbg";
    static inline std::string SubfolderName = "";
    static inline std::string ModuleName = "";

    static inline fs::path MainFolder;
    static inline fs::path Subfolder;

private:
    using StreamType = std::ofstream;

private:
    template<typename InStreamType, typename T>
    static void WriteToStream(InStreamType& InStream, T Value)
    {
        InStream.write(reinterpret_cast<const char*>(&Value), sizeof(T));
    }

    template<typename InStreamType, typename T>
    static void WriteToStream(InStreamType& InStream, T* Value, int32 Size)
    {
        InStream.write(reinterpret_cast<const char*>(Value), Size);
    }

private:
    static void GenerateVTableName(nlohmann::json& jsonArray, UEObject DefaultObject);
    static void GenerateClassFunctions(nlohmann::json& jsonArray, UEClass Class);
    static void WriteGlobal(nlohmann::json& jsonArray, const char* Name, int32 RVA);

public:
    static void Generate();

    static void InitPredefinedMembers() {}
    static void InitPredefinedFunctions() {}
};
#include <pch.h>

#include <model/studio/studio.h>
#include <model/internalmodel/internalmodel.h>
#include <model/internalmodel/internalmodel_extract.h>

CmdSettings_t g_CmdSettings;
CBufferManager g_BufferManager;


void ExtractStudioFile(const char* fileDir)
{
	std::vector<std::string> paths;

	StudioPathsFromDirectory(fileDir, paths);

	std::filesystem::path outpath(g_CmdSettings.outpath ? g_CmdSettings.outpath : fileDir);
	
	// check for an extension here and shorten it to a parent path if it has one, we don't check has_filename() as inproperly formated paths could give false positives
	if (outpath.has_extension())
	{
		outpath = outpath.parent_path();
	}

	const char* const dirEnd = keepAfterLastSlashOrBackslash(fileDir);
	const size_t dirLen = dirEnd - fileDir; // used to get the local model path
	for (const std::string& path : paths)
	{
		Log("extracting files from %s\n", path.c_str());

		// sanity check
		if (!std::filesystem::exists(path))
		{
			Log("skipped %s because it didn't exist.\n", path.c_str());
			continue;
		}

		IModel* iModel = IModel::LoadStudioModel(path);

		// skip if nullptr (hasn't been implemented)
		if (!iModel)
		{
			Log("skipped %s because it was unimplemented.\n", path.c_str());
			continue;
		}

		// skip if faild to parse
		if (iModel->sourceType == IModelSourceType_t::SOURCE_FAILED)
		{
			Log("skipped %s because it failed to parse.\n", path.c_str());
			FreeAllocVar(iModel);
			continue;
		}

		// build the output directory
		std::filesystem::path localDir(path.substr(dirLen, path.length())); // local path to our model, declare pre-emptively to ease readability
		const std::string outName = localDir.stem().string();				// save file name
		localDir = localDir.parent_path();		// sans filename
		localDir = localDir.relative_path();	// removes drive and or bad formatting, example: "file/out/models" (bad) vs "file/out/models/" (good)

		std::filesystem::path outDir(outpath);	// output directory with outpath as a base
		outDir /= localDir;						// append the local directory of our studio file on to outDir

		// added 'parent_path()' here otherwise it was attempting to create a blank directory if the path ended in a forward slash
		if (!CreateDirectories(outDir.parent_path()))
		{
			Log("failed to create directory for %s, skipping...\n", outName.c_str());
			FreeAllocVar(iModel);

			assertm(false, "failed to create directory");
			
			continue;
		}

		IModelExporter imodelExport(iModel, outDir, outName);
		imodelExport.ParseForExport();

		Log("successfully extracted files!\n");

		FreeAllocVar(iModel);
	}
}

// parse the command line into g_CmdSettings
inline void ParseCommandSettings(const CmdLine& cmd)
{
	using namespace smd;
	using namespace qc;

	// parse -outpath command
	if (cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_OUTPATH]))
	{
		const char* const outpath = cmd.GetArgValue(s_CommandArgs[eCmdArgs::CMD_OUTPATH]);
		g_CmdSettings.outpath = outpath;

		if (g_CmdSettings.outpath == nullptr)
		{
			Log("Command %s used without value\n", s_CommandArgs[eCmdArgs::CMD_OUTPATH]);
		}
	}

	// parse -format command
	if (cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_FORMAT]))
	{
		const uint32_t iFmt = cmd.GetArgUInt(s_CommandArgs[eCmdArgs::CMD_FORMAT]);
		g_CmdSettings.format = iFmt < eModelExportFormat::MODEL_FMT_COUNT ? iFmt : eModelExportFormat::MODEL_FMT_INVALID;

		if (g_CmdSettings.format == eModelExportFormat::MODEL_FMT_INVALID)
		{
			Log("Command %s had invalid input of %u\n", s_CommandArgs[eCmdArgs::CMD_FORMAT], iFmt);
		}
	}

	// parse -version command
	if (cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_VERSION]))
	{
		const char* const version = cmd.GetArgValue(s_CommandArgs[eCmdArgs::CMD_VERSION]);
		g_CmdSettings.version = version;

		if (g_CmdSettings.outpath == nullptr)
		{
			Log("Command %s used without value\n", s_CommandArgs[eCmdArgs::CMD_VERSION]);
		}
	}

	// parse -mergeuiverts command
	if (cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_MERGEUIVERTS]))
	{
		const uint8_t value = static_cast<uint8_t>(cmd.GetArgUInt(s_CommandArgs[eCmdArgs::CMD_MERGEUIVERTS]));
		g_CmdSettings.mergeUIVerts = value < 15u ? value : 15u;
	}

	// parse -qc_version command
	if (cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_QC_VERSION]))
	{
		const int out = sscanf_s(cmd.GetArgValue(s_CommandArgs[eCmdArgs::CMD_QC_VERSION]), "%hu %hu", reinterpret_cast<uint16_t*>(&g_CmdSettings.qcVersion.major), &g_CmdSettings.qcVersion.minor);
		if (out < 1)
		{
			Error("Failed to read values from %s command! Exiting...\n", s_CommandArgs[eCmdArgs::CMD_QC_VERSION]);
		}

		if (out == 1)
		{
			printf("Command %s only had %i value(s) when expecting %i, this is not recommended!\n", s_CommandArgs[eCmdArgs::CMD_QC_VERSION], out, 2);
		}
	}

	// parse -smd_version command
	if (cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_SMD_VERSION]))
	{
		const uint32_t versionSMD = cmd.GetArgInt(s_CommandArgs[eCmdArgs::CMD_SMD_VERSION]);
		g_CmdSettings.smdVersion = versionSMD <= SMD_V3 ? versionSMD : SMD_INVALID;

		if (g_CmdSettings.smdVersion == SMD_INVALID)
		{
			Log("Command %s had invalid input of %u\n", s_CommandArgs[eCmdArgs::CMD_SMD_VERSION], versionSMD);
		}
	}

	// parse -dmx_version command
	if (cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_DMX_VERSION]))
	{
		g_CmdSettings.dmxVersionSource = cmd.GetArgValue(s_CommandArgs[eCmdArgs::CMD_DMX_VERSION]);

		if (g_CmdSettings.dmxVersionSource == nullptr)
		{
			Log("Command %s had no input\n", s_CommandArgs[eCmdArgs::CMD_DMX_VERSION]);
		}
		else
		{
			g_CmdSettings.dmxVersion.FromString(g_CmdSettings.dmxVersionSource);

			if (g_CmdSettings.dmxVersion.IsValid() == false)
			{
				Log("Command %s had invalid input of %s\n", s_CommandArgs[eCmdArgs::CMD_DMX_VERSION], g_CmdSettings.dmxVersionSource);
				g_CmdSettings.dmxVersion = DmVersion_t(DmVersion_t::DMX_ENCODE_BINARY, DmVersion_t::DMX_FORMAT_MODEL, 5, 18); // set a default version so things don't break!
			}
		}
	}

	// toggle settings
	g_CmdSettings.qcWrite = cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_QC_WRITE]);
	g_CmdSettings.qcUseIncludes = cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_QC_INCLUDES]);
	g_CmdSettings.qcUseTrimmedSkins = cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_QC_TRIMSKINS]);

	g_CmdSettings.truncateMaterials = cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_TRUNCATEMATERIALS]);
	g_CmdSettings.ignoreMotion = cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_IGNOREMOTION]);
	g_CmdSettings.ignoreMesh = cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_IGNOREMESH]);
	g_CmdSettings.ignoreAnim = cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_IGNOREANIM]);
}

int main(int argc, char** argv)
{
	// error because we need args
	if (argc < 2)
		Error("No arguments provided, exiting...\n");

	CmdLine cmd = CmdLine(argc, argv); // handle our arguments

	ParseCommandSettings(cmd);

	if (cmd.HasArg(s_CommandArgs[eCmdArgs::CMD_EXTRACT]))
	{
		//system("pause");

		const char* inPath = cmd.GetArgValue(s_CommandArgs[eCmdArgs::CMD_EXTRACT]);

		if (!inPath)
		{
			Error("Command '%s' used without a value, this is not the proper usage!!\n", s_CommandArgs[eCmdArgs::CMD_EXTRACT]);
		}

		CScopeTimer funcTime("Extracting Studio File(s)");

		ExtractStudioFile(inPath);

		//system("pause");

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

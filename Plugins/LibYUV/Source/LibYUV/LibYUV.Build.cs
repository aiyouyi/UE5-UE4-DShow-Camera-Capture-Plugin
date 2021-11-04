// Some copyright should be here...

using UnrealBuildTool;
using System;
using System.IO;



public class LibYUV : ModuleRules
{

    private string LibPath
    {
        get
        {
			return Path.GetFullPath(Path.Combine(PluginDirectory, "ThirdParty", "LIB"));
        }
    }

    public string GetUProjectPath()
    {
        return Path.Combine(ModuleDirectory, "../../../..");
    }


    private int HashFile(string FilePath)
    {
        string DLLString = File.ReadAllText(FilePath);
        return DLLString.GetHashCode() + DLLString.Length;  //ensure both hash and file lengths match
    }

    private void CopyToProjectBinaries(string Filepath, ReadOnlyTargetRules Target)
    {
        string BinariesDir = Path.Combine(GetUProjectPath(), "Binaries", Target.Platform.ToString());
        string Filename = Path.GetFileName(Filepath);

        //convert relative path 
        string FullBinariesDir = Path.GetFullPath(BinariesDir);

        if (!Directory.Exists(FullBinariesDir))
        {
            Directory.CreateDirectory(FullBinariesDir);
        }

        string FullExistingPath = Path.Combine(FullBinariesDir, Filename);
        bool ValidFile = false;

        //File exists, check if they're the same
        if (File.Exists(FullExistingPath))
        {
            int ExistingFileHash = HashFile(FullExistingPath);
            int TargetFileHash = HashFile(Filepath);
            ValidFile = ExistingFileHash == TargetFileHash;
            if (!ValidFile)
            {
            }
        }
        if (!ValidFile)
        {
            File.Copy(Filepath, Path.Combine(FullBinariesDir, Filename), true);
        }
    }

    public LibYUV(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        PublicAdditionalLibraries.AddRange(new string[] {
            Path.Combine(LibPath, "yuv.lib"),
            Path.Combine(LibPath, "jpeg.lib"),
            Path.Combine(LibPath, "jpeg-static.lib"),
            Path.Combine(LibPath, "turbojpeg.lib"),
            Path.Combine(LibPath, "turbojpeg-static.lib"),
        });

        Definitions.Add("HAVE_JPEG");

        PublicSystemIncludePaths.Add(Path.Combine(PluginDirectory, "ThirdParty", "Include"));

        string PlatformString = Target.Platform.ToString();
        CopyToProjectBinaries(Path.GetFullPath(Path.Combine(LibPath, "jpeg62.dll")), Target);
        RuntimeDependencies.Add(Path.GetFullPath(Path.Combine(GetUProjectPath(), "Binaries", PlatformString, "jpeg62.dll")));
        CopyToProjectBinaries(Path.GetFullPath(Path.Combine(LibPath, "libyuv.dll")), Target);
        RuntimeDependencies.Add(Path.GetFullPath(Path.Combine(GetUProjectPath(), "Binaries", PlatformString, "libyuv.dll")));
        CopyToProjectBinaries(Path.GetFullPath(Path.Combine(LibPath, "turbojpeg.dll")), Target);
        RuntimeDependencies.Add(Path.GetFullPath(Path.Combine(GetUProjectPath(), "Binaries", PlatformString, "turbojpeg.dll")));
    }
}

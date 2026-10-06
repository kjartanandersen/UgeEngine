-- premake's .slnx writer (vs2026) only maps a project's platform when `configmap` names one,
-- and configmap only accepts names listed in `platforms`, which this workspace doesn't use
-- (it sets `architecture` instead). With no mapping, MSBuild and Visual Studio build every
-- C# project as "Any CPU", a platform the generated .csproj has no configuration for:
-- "The BaseOutputPath/OutputPath property is not set". The .sln writer always wrote these
-- mappings, which is why Uge.sln worked. C++ projects are unaffected.
require('vstudio')

premake.override(premake.vstudio.sln2026.elements, "project", function(base, prj)
	local calls = base(prj)
	table.insert(calls, function(prj)
		if not premake.project.isdotnet(prj) then
			return
		end
		-- Every configuration has the workspace's one architecture, so one mapping covers them all.
		for cfg in premake.project.eachconfig(prj) do
			premake.w('<Platform Project="%s" />', premake.vstudio.archFromConfig(cfg))
			break
		end
	end)
	return calls
end)
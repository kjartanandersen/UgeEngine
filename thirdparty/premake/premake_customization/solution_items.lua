-- Implement the solution_items command for solution-scope files
require('vstudio')

premake.api.register {
	name = "solution_items",
	scope = "workspace",
	kind = "list:string",
}

premake.override(premake.vstudio.sln2005, "projects", function(base, wks)
	if wks.solution_items and #wks.solution_items > 0 then
		local solution_folder_GUID = "{2150E333-8FDC-42A3-9474-1A3956D46DE8}" -- See https://www.codeproject.com/Reference/720512/List-of-Visual-Studio-Project-Type-GUIDs
		premake.push("Project(\"" .. solution_folder_GUID .. "\") = \"Solution Items\", \"Solution Items\", \"{" .. os.uuid("Solution Items:" .. wks.name) .. "}\"")
		premake.push("ProjectSection(SolutionItems) = preProject")

		for _, path in ipairs(wks.solution_items) do
			premake.w(path .. " = " .. path)
		end

		premake.pop("EndProjectSection")
		premake.pop("EndProject")
	end
	base(wks)
end)

-- The same for the .slnx (vs2026) writer, where solution items are a folder of <File> entries.
premake.override(premake.vstudio.sln2026.elements, "solution", function(base, wks)
	local calls = base(wks)
	table.insert(calls, function(wks)
		if wks.solution_items and #wks.solution_items > 0 then
			premake.push('<Folder Name="/Solution Items/">')
			for _, item in ipairs(wks.solution_items) do
				premake.w('<File Path="%s" />', item)
			end
			premake.pop('</Folder>')
		end
	end)
	return calls
end)

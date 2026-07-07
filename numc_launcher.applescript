-- Numc.app — Desktop launcher for numc.py
-- Per-machine artifact (absolute repo path); regenerate with osacompile if
-- the repo moves. Launches the solver GUI detached; if one is already running,
-- brings its window to the front instead of starting a second instance
-- (needs a one-time Automation consent for System Events).

on run
	set repoDir to "/Users/roman/projects/numc"
	try
		-- anchored pattern: match the python GUI process itself, not its sh wrapper
		set foundPid to do shell script "pgrep -f 'numc\\.py$' | head -n 1; true"
		if foundPid is not "" then
			try
				tell application "System Events" to set frontmost of (first process whose unix id is (foundPid as integer)) to true
			on error
				display notification "numc is already running" with title "Numc"
			end try
		else
			do shell script "cd " & quoted form of repoDir & " && nohup /opt/homebrew/bin/python3 numc.py > /dev/null 2>&1 &"
		end if
	on error errMsg number errNum
		display dialog "Numc launch failed (" & errNum & "): " & errMsg buttons {"OK"} default button 1 with icon stop with title "Numc"
	end try
end run

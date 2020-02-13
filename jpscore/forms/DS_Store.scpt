on run argv
	-- Constants
	set X_POS to 400
	set Y_POS to 100
	set BG_W to 650
	set BG_H to 400
	set TITLE_BAR_H to 15

	set diskImage to item 1 of argv

	tell application "Finder"
		tell disk diskImage
			-- Setup background and icon arrangement
			open
			set current view of container window to icon view
			set theViewOptions to the icon view options of container window
			set background picture of theViewOptions to file ".background:background.png"
			set arrangement of theViewOptions to not arranged
			set icon size of theViewOptions to 72
			delay 5
			close

			open
			update without registering applications
			tell container window
				set sidebar width to 0
				set statusbar visible to false
				set toolbar visible to false
				set the bounds to {X_POS, Y_POS, X_POS + BG_W, Y_POS + BG_H + TITLE_BAR_H}
                                -- x  y
				set position of item "bin" to {0, 170}
                                set position of item "jpscore_samples" to {150, 170}
                                set position of item "jpsreport_samples" to {300, 170}
                                set position of item "LICENSE" to {0, 280}
                                set position of item "README.md" to {150, 280}
                                set position of item "JuPedSim.pdf" to {300, 280}
				set position of item ".background" to {300, 280}
				set position of item ".fseventsd" to {400, 280}
			end tell
			update without registering applications
			delay 5
			close
			-- Show window one more time for a final check
			open
			delay 5
			close
		end tell
		delay 1
	end tell
end run

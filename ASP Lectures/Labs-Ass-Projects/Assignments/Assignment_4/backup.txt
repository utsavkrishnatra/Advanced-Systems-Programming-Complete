## Utsav Krishnatra
## Student No-110095341
## Sec- 2

# Function for determining backup parameters and performing backup operations.
function ut_gimmeNumbers {
    # Search for the most recent CompleteBackup tar file in the cb directory.
    ut_mysterioParam=$(ls -1t ~/home/backup/cb/cb*.tar 2>/dev/null | head -n1)
    
    if [ -n "$ut_mysterioParam" ]; then
        # Extract the number from the CompleteBackup tar file name and increment it.
        ut_superBigger=$(echo "$ut_mysterioParam" | sed 's/.*cb\([0-9]*\).tar/\1/')
        ((ut_superBigger++))
    else
        # If no CompleteBackup tar files found, start with a default value.
        ut_superBigger=20001
    fi

    # Search for the most recent IncrementalBackup tar file in the ib directory.
    ut_srchIncBkp=$(ls -1t ~/home/backup/ib/ib*.tar 2>/dev/null | head -n1)
    
    if [ -n "$ut_srchIncBkp" ]; then
        # Extract the number from the IncrementalBackup tar file name and increment it.
        ut_jackpotIncBkp=$(echo "$ut_srchIncBkp" | sed 's/.*ib\([0-9]*\).tar/\1/')
        ((ut_jackpotIncBkp++))
    else
        # If no IncrementalBackup tar files found, start with a default value.
        ut_jackpotIncBkp=10001
    fi
}

# Function for creating a complete backup of text files.
function ut_createCompleteBkp {
    # Create a tar file containing all text files in the user's home directory.
    tar -cf ~/home/backup/cb/cb"${ut_superBigger}".tar $(find "$HOME" -type f -name "*.txt") 2>/dev/null
    # Log the creation of the CompleteBackup tar file.
    echo "$(date) - cb${ut_superBigger}.tar has been generated" >> ~/home/backup/backup.log
    # Increment the CompleteBackup counter.
    ((ut_superBigger++))
}

# Function for creating incremental backups.
function ut_createIncrementalBkp {
    # Determine the CompleteBackup tar file from the previous iteration.
    ut_magicalTar=~/home/backup/cb/cb"$((ut_superBigger-1))".tar
    
    if [ "$ut_jackpotIncBkp" -gt 10001 ]; then
        # Determine the previous IncrementalBackup tar file.
        ut_secretTar=~/home/backup/ib/ib"$((ut_jackpotIncBkp-1))".tar
        # Find text files modified since both CompleteBackup and IncrementalBackup.
        ut_modifiedScrolls=$(find "$HOME" -type f -name "*.txt" -newer "$ut_magicalTar" -newer "$ut_secretTar" 2>/dev/null)
    else
        # Find text files modified since only the CompleteBackup.
        ut_modifiedScrolls=$(find "$HOME" -type f -name "*.txt" -newer "$ut_magicalTar" 2>/dev/null)
    fi
    
    if [ -n "$ut_modifiedScrolls" ]; then
        # Create an IncrementalBackup tar file with modified text files.
        tar -cf ~/home/backup/ib/ib"${ut_jackpotIncBkp}".tar $ut_modifiedScrolls
        # Log the creation of the IncrementalBackup tar file.
        echo "$(date) - ib${ut_jackpotIncBkp}.tar was created" >> ~/home/backup/backup.log
        # Increment the IncrementalBackup counter.
        ((ut_jackpotIncBkp++))
    else
        # Log the absence of changes for the IncrementalBackup.
        echo "$(date) No changes - Incremental backup idle" >> ~/home/backup/backup.log
    fi
}

# The infinite loop for backup operations.
while true; do
    # Create the IncrementalBackup directory if it doesn't exist.
    if [ ! -d ~/home/backup/ib ]; then
        mkdir -p ~/home/backup/ib
    fi
    
    # Create the CompleteBackup directory if it doesn't exist.
    if [ ! -d ~/home/backup/cb ]; then
        mkdir -p ~/home/backup/cb
    fi
    
    # Determine backup parameters.
    ut_gimmeNumbers
    
    # Create a CompleteBackup.
    ut_createCompleteBkp
    
    # Wait for 2 minutes.
    sleep 120
    
    # Create three rounds of IncrementalBackup with 2 minutes interval each.
    for (( p = 0; p < 3; p++ )); do
        ut_createIncrementalBkp
        sleep 120
    done
done
# urage
A custom database for well-oriented performance and user experience. 

# Windows build

# 1. Navigate to project directory
cd D:\Desktop\urage

# 2. Create necessary directories
mkdir obj, bin, data -Force

# 3. Compile the database
gcc -Wall -Wextra -g -I./src src/*.c -o bin/database.exe

# 4. Run the database
.\bin\database.exe data\mydb

# Alternative: Run with custom database name
.\bin\database.exe data\customername

# 5. Clean build (if needed)
Remove-Item obj\* -Force
Remove-Item bin\* -Force


# Linux ( Ubuntu ) build

# 1. Install GCC (if not installed)
sudo apt update
sudo apt install gcc make

# 2. Navigate to project directory
cd ~/urage

# 3. Create necessary directories
mkdir -p obj bin data

# 4. Compile the database
gcc -Wall -Wextra -g -I./src src/*.c -o bin/database

# 5. Run the database
./bin/database data/mydb

# Alternative: Run with custom database name
./bin/database data/customername

# 6. Clean build (if needed)
rm -f obj/* bin/*
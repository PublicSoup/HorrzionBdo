@echo off
echo ========================================
echo   Upload BDO Bot to GitHub
echo ========================================
echo.

echo This script will help you upload your code to GitHub.
echo.
echo BEFORE RUNNING THIS:
echo   1. Create a GitHub account at github.com
echo   2. Install GitHub Desktop from desktop.github.com
echo   OR
echo   2. Install Git from git-scm.com
echo.
echo Press any key to continue, or close this window to exit...
pause >nul

echo.
echo ========================================
echo   Step 1: Check Git Installation
echo ========================================
echo.

where git >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Git is not installed!
    echo.
    echo Please install Git from:
    echo https://git-scm.com/download/win
    echo.
    echo OR use GitHub Desktop:
    echo https://desktop.github.com/
    echo.
    pause
    exit /b 1
)

echo ✓ Git is installed!
git --version

echo.
echo ========================================
echo   Step 2: Configure Git
echo ========================================
echo.

git config --global user.name >nul 2>nul
if %errorlevel% neq 0 (
    echo Setting up Git configuration...
    echo.
    set /p USERNAME="Enter your name: "
    set /p EMAIL="Enter your email: "
    git config --global user.name "%USERNAME%"
    git config --global user.email "%EMAIL%"
)

echo ✓ Git configured!
echo   Name: 
git config --global user.name
echo   Email: 
git config --global user.email

echo.
echo ========================================
echo   Step 3: Initialize Repository
echo ========================================
echo.

if exist ".git" (
    echo Repository already initialized!
) else (
    echo Initializing Git repository...
    git init
    echo ✓ Repository initialized!
)

echo.
echo ========================================
echo   Step 4: Check .gitignore
echo ========================================
echo.

if exist ".gitignore" (
    echo ✓ .gitignore file exists!
) else (
    echo WARNING: .gitignore not found!
    echo Creating default .gitignore...
    (
        echo # Build outputs
        echo bin/
        echo obj/
        echo *.exe
        echo *.dll
        echo *.pdb
        echo.
        echo # Visual Studio
        echo .vs/
        echo *.user
        echo.
        echo # Personal files
        echo my_addresses.txt
        echo *.CT
    ) > .gitignore
    echo ✓ .gitignore created!
)

echo.
echo ========================================
echo   Step 5: Add Files to Git
echo ========================================
echo.

echo Adding all files...
git add .

echo.
echo Files to be committed:
git status --short

echo.
echo ========================================
echo   Step 6: Create First Commit
echo ========================================
echo.

git log --oneline -1 >nul 2>nul
if %errorlevel% neq 0 (
    echo Creating first commit...
    git commit -m "Initial commit: BDO Stealth Bot with advanced memory scanning"
    echo ✓ First commit created!
) else (
    echo Repository already has commits.
    echo.
    set /p COMMIT_MSG="Enter commit message (or press Enter to skip): "
    if not "%COMMIT_MSG%"=="" (
        git commit -m "%COMMIT_MSG%"
        echo ✓ Changes committed!
    ) else (
        echo Skipping commit...
    )
)

echo.
echo ========================================
echo   Step 7: Create GitHub Repository
echo ========================================
echo.

echo Please create a repository on GitHub:
echo   1. Go to https://github.com/new
echo   2. Name: BDO-Stealth-Bot
echo   3. Description: Advanced memory scanning and bot automation for BDO
echo   4. Choose: Public or Private
echo   5. Do NOT initialize with README
echo   6. Click "Create repository"
echo.
echo Have you created the repository? (y/n)
set /p CREATED=
if /i not "%CREATED%"=="y" (
    echo.
    echo Please create the repository first, then run this script again.
    pause
    exit /b 0
)

echo.
echo ========================================
echo   Step 8: Add Remote Repository
echo ========================================
echo.

set /p GITHUB_USERNAME="Enter your GitHub username: "
set /p REPO_NAME="Enter repository name (default: BDO-Stealth-Bot): "
if "%REPO_NAME%"=="" set REPO_NAME=BDO-Stealth-Bot

set REPO_URL=https://github.com/%GITHUB_USERNAME%/%REPO_NAME%.git

echo.
echo Repository URL: %REPO_URL%
echo.

git remote get-url origin >nul 2>nul
if %errorlevel% equ 0 (
    echo Remote 'origin' already exists. Updating...
    git remote set-url origin %REPO_URL%
) else (
    echo Adding remote repository...
    git remote add origin %REPO_URL%
)

echo ✓ Remote repository configured!

echo.
echo ========================================
echo   Step 9: Push to GitHub
echo ========================================
echo.

echo Pushing code to GitHub...
echo.
echo NOTE: You may be prompted for GitHub credentials.
echo       Use Personal Access Token, not password!
echo.
echo Creating/updating main branch...
git branch -M main

echo.
echo Pushing to GitHub...
git push -u origin main

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo   ✓ SUCCESS!
    echo ========================================
    echo.
    echo Your code is now on GitHub!
    echo.
    echo View it at:
    echo https://github.com/%GITHUB_USERNAME%/%REPO_NAME%
    echo.
    echo Next steps:
    echo   1. Add topics/tags to your repository
    echo   2. Edit README.md if needed
    echo   3. Create a release (optional)
    echo   4. Share with others!
    echo.
) else (
    echo.
    echo ========================================
    echo   ⚠ PUSH FAILED
    echo ========================================
    echo.
    echo Common issues:
    echo   1. Incorrect username/repository name
    echo   2. Repository doesn't exist on GitHub
    echo   3. Need to use Personal Access Token
    echo.
    echo To create Personal Access Token:
    echo   1. Go to GitHub → Settings → Developer Settings
    echo   2. Personal Access Tokens → Generate new token
    echo   3. Select scopes: repo
    echo   4. Use token as password when prompted
    echo.
)

echo.
echo ========================================
echo   Quick Reference
echo ========================================
echo.
echo Update repository later:
echo   git add .
echo   git commit -m "Your message"
echo   git push
echo.
echo View repository:
echo   https://github.com/%GITHUB_USERNAME%/%REPO_NAME%
echo.

pause

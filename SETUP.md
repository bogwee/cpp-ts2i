# Git & GitHub Setup

This document explains how to set up Git for your machine learning project, connect it to GitHub, configure authentication properly, and push your code from the terminal.

---

## 🔧 Prerequisites

Before you begin, make sure:

* `git` is installed → check with: `git --version`
* You have a [GitHub account](https://github.com)
* Authentication is set up:

  * **Option 1 (Quick):** HTTPS with a [Personal Access Token (PAT)](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/creating-a-personal-access-token)
  * **Option 2 (Recommended):** SSH with your public key added to GitHub

---

## 🚀 Step-by-Step Setup

### 1. Navigate to Your Project Folder

```bash
cd path/to/your-project
```

### 2. Initialize a Git Repository

```bash
git init
```

### 3. Set Your Git Identity

Make sure commits show as coming from **you**:

```bash
git config --global user.name "Your Name"
git config --global user.email "your_github_email@example.com"
```

### 4. Add Files and Make Your First Commit

```bash
git add .
git commit -m "Initial commit"
```

### 5. Create a GitHub Repository

* Go to [https://github.com/new](https://github.com/new)
* Name your repository (e.g., `your-project`)
* Don’t check "Initialize with README" if your local repo already has one
* Click **Create repository**

### 6. Connect Local Repo to GitHub

```bash
git remote add origin https://github.com/YOUR_USERNAME/your-project.git
```

To check your branch name:

```bash
git branch --show-current
```

---

## 🔐 Authentication Setup

### SSH Keys

1. Generate an SSH key:

```bash
ssh-keygen -t ed25519 -C "your_github_email@example.com"
```

3. Copy your public key:

```bash
cat ~/.ssh/id_ed25519.pub
```

4. Add it to GitHub: **Settings → SSH and GPG keys → New SSH key**
5. Connect it to Github:

```bash
git remote set-url origin git@github.com:username/repo.git
```

Then push:

```bash
git push -u origin master
```

---

## 🔁 Daily Git Workflow

1. Stage changes:

```bash
git add .
```

2. Commit:

```bash
git commit -m "Day X: Work description"
```

3. Pull updates:

```bash
git pull origin main
```

4. Push:

```bash
git push
```
# Push and Open Pull Request

Your branch `feature/agent-manager-tls-nis2` is created and committed. Follow these steps to push and open a PR.

## 1. Fork the Wazuh repo (if you haven't already)

Go to https://github.com/wazuh/wazuh and click **Fork**. This creates a copy under your GitHub account.

## 2. Add your fork and push

Replace `YOUR_GITHUB_USERNAME` with your actual GitHub username:

```powershell
cd c:\Users\swats\Desktop\SOC\wazuh-tls

git remote add myfork https://github.com/YOUR_GITHUB_USERNAME/wazuh.git
git push -u myfork feature/agent-manager-tls-nis2
```

## 3. Open the Pull Request

1. Go to https://github.com/wazuh/wazuh/compare
2. **Base repository:** wazuh/wazuh, **base:** main
3. **Head repository:** YOUR_GITHUB_USERNAME/wazuh, **compare:** feature/agent-manager-tls-nis2
4. Click **Create pull request**
5. Copy the content from `PR_DESCRIPTION.md` into the PR description
6. Submit

## 4. Optional: Run tests before submitting

```bash
# Build (from project root or src/)
cd src && make TARGET=manager
cd .. && make TARGET=agent

# Run remoted integration tests (if pytest is available)
cd tests/integration && pytest test_remoted/ -v -k "socket or valid" --tb=short
```

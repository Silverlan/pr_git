#include <pragma/pragma_module.hpp>
#include <git2.h>
#include <git2/clone.h>
#include <git2/filter.h>
#include <string>
#include <cstring>
#include <vector>
#include <memory>
#include <sharedutils/scope_guard.h>

static bool check_error(int err,std::string &outErr)
{
	if(err >= 0)
		return true;
	auto *gitErr = git_error_last();
	if(!gitErr)
		outErr = "Unknown error";
	else
		outErr = gitErr->message;
	return false;
}

static git_commit *get_last_commit(git_repository *repo)
{
	int rc;
	git_commit *commit = nullptr; /* the result */
	git_oid oid_parent_commit; /* the SHA1 for last commit */

	/* resolve HEAD into a SHA1 */
	rc = git_reference_name_to_id(&oid_parent_commit,repo,"HEAD");
	if(rc == 0)
	{
		/* get the actual commit structure */
		rc = git_commit_lookup(&commit,repo,&oid_parent_commit);
		if(rc == 0)
			return commit;
	}
	return nullptr;
}

extern "C"
{
	bool PRAGMA_EXPORT pr_git_clone(
		const std::string &repositoryUrl,const std::string &branch,
		const std::vector<std::string> &filter,const std::string &outputDir,std::string &outErr,
		std::string *optOutCommitId
	)
	{
		auto err = git_libgit2_init();
		if(check_error(err,outErr) == false)
			return false;
		util::ScopeGuard sgLibgit2 {[]() {
			git_libgit2_shutdown();
		}};
		auto &url = repositoryUrl;

		git_clone_options options {};
		git_clone_options_init(&options,GIT_CLONE_OPTIONS_VERSION);

		struct PathFilter
		{
			~PathFilter()
			{
				for(auto *p : m_data)
					delete[] p;
			}
			void AddPath(const std::string &path) {m_paths.push_back(path);}
			char **GetPaths(uint32_t &n)
			{
				n = m_paths.size();
				if(!m_data.empty())
					return m_data.data();
				m_data.resize(m_paths.size());
				for(auto i=decltype(m_paths.size()){0u};i<m_paths.size();++i)
				{
					auto &path = m_paths[i];
					auto sz = path.length() +1;
					m_data[i] = new char[sz];
#ifdef _WIN32
					strcpy_s(m_data[i],sz,path.data());
#else
					strcpy(m_data[i],path.data());
#endif
				}
				return m_data.data();
			}
		private:
			std::vector<char*> m_data;
			std::vector<std::string> m_paths;
		};

		PathFilter pathFilter {};
		for(auto &f : filter)
			pathFilter.AddPath(f);

		uint32_t n;
		auto *p = pathFilter.GetPaths(n);

		options.checkout_branch = branch.c_str();
		options.checkout_opts.paths.count = n;
		options.checkout_opts.paths.strings = p;
		
		git_repository *prepo = nullptr;
		auto ret = git_clone(&prepo,url.c_str(),outputDir.c_str(),&options);
		std::unique_ptr<git_repository,void(*)(git_repository*)> repo {prepo,[](git_repository *repo) {
			if(!repo)
				return;
			git_repository_free(repo);
		}};
		if(check_error(err,outErr) == false)
			return false;
		if(!repo)
		{
			auto *gitErr = git_error_last();
			if(!gitErr)
				outErr = "Unknown error";
			else
				outErr = gitErr->message;
			return false;
		}

		if(optOutCommitId)
		{
			*optOutCommitId = "";
			auto commit = std::unique_ptr<git_commit,void(*)(git_commit*)>(get_last_commit(repo.get()),[](git_commit *c) {
				if(!c)
					return;
				git_commit_free(c);
			});
			auto *id = commit ? git_commit_id(commit.get()) : nullptr;
			if(id)
			{
				constexpr uint32_t len = 40;
				char hex[len];
				ret = git_oid_fmt(hex,id);
				std::string err;
				if(check_error(ret,err))
					*optOutCommitId = std::string{hex,len};
			}
			commit = nullptr;
		}

		repo = nullptr;
		return true;
	}
};

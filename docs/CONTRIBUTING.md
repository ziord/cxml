### Welcome

Hi, and welcome to the contribution guideline page. Thanks for taking the effort to get here. Consider taking a look at the [project architecture](https://github.com/ziord/cxml/blob/master/docs/ARCHITECTURE.md) if you haven't already.
Below are the steps that needs to be followed in order to contribute to this project.

### Fork the Project
    
You can follow [this guideline](https://docs.github.com/en/github/getting-started-with-github/fork-a-repo) to fork the cxml project. 


### Add the Feature

Add the feature you'd like to contribute to the project. The project has been organized into folders for modularity, with names highlighting the items expected to be found in each.
Please keep commits cohesive, and simple enough to describe the change being added.  


### Add a Test

Please add a test for the newly added feature.
For instance if you add a feature **A** in `src/core` folder, then you should add a test in `tests/core` folder. The test function(s) should be added to the source file corresponding to the file that the feature was added.
That is, if feature **A** was added to `src/core/cxlist.c`, then the corresponding test function(s) should be added to `tests/core/cxlist_test.c`
Please ensure that the test actually tests the feature added. Also, run the tests to ensure that the test passes, and older tests are not broken.

cxml uses its own (header-only) test library, that is fairly intuitive and easy to use. You can check out previous tests added as a guide, and even the test library file itself (`cxtest.h`) to know how it works. If there's anything unclear about this, please open an issue or use the project's discussions. The test library would hopefully be documented soon. 


### Open a Pull Request

Create a pull request for the change added. [This guideline](https://docs.github.com/en/github/collaborating-with-issues-and-pull-requests/creating-a-pull-request) details how to do so.
Ensure to add a title that states the specific purpose of the pull request. For bug fixes, in the description, ensure to add a break down of the problem, reproducibility, and how your pull request fixes the problem. For feature addition, ensure to add a break down of the feature, including its purpose/motivation/reason.


### Code Style

Embrace simplicity over tricky, complicated approach of solving a problem. Typical code lines should be 70-100 lines long, only longer for comments.
Adding comments to sections that wouldn't be immediately clear is recommended, and encouraged.

PS: write pretty code. 😃  

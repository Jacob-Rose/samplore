#include "DirectoryExplorer.h"
#include "SamplifyProperties.h"
#include "Icons.h"

using namespace samplore;

DirectoryExplorer::DirectoryExplorer()
{
	setScrollBarsShown(true, true, true, true);
	addAndMakeVisible(mDirectoryTree);
	SamplifyProperties::getInstance()->getSampleLibrary()->addChangeListener(this);
	refresh();
}

DirectoryExplorer::~DirectoryExplorer()
{
	if (auto* lib = SamplifyProperties::getInstance()->getSampleLibrary().get())
		lib->removeChangeListener(this);
	mDirectoryTree.deleteRootItem();
}


void DirectoryExplorer::paint (Graphics& g)
{
    
}

void DirectoryExplorer::resized()
{
	mDirectoryTree.setBounds(getLocalBounds());
}

void DirectoryExplorer::refresh()
{
	TreeViewItem* root = mDirectoryTree.getRootItem();
	if (root == nullptr)
	{
		root = new DirectoryExplorerTreeViewItem("All Directories");
		mDirectoryTree.setRootItem(root);
	}
	else
	{
		root->clearSubItems();
	}
	const auto& dirs = SamplifyProperties::getInstance()->getSampleLibrary()->getDirectories();
	for (int i = 0; i < dirs.size(); i++)
	{
		DirectoryExplorerTreeViewItem* item = new DirectoryExplorerTreeViewItem(dirs[i]);
		root->addSubItem(item);
	}
	root->setSelected(true, true);
}

void DirectoryExplorer::changeListenerCallback(ChangeBroadcaster* source)
{
	refresh();
}

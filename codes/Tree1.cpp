#include <iostream>
#include <stack>
using namespace std;

template <typename T>
struct BiTNode
{
  T data;
  BiTNode<T> *lchild;
  BiTNode<T> *rchild;
};

// Create tree from preorder input, use '#' to denote null.
// Important: pass Tree by reference so caller pointer is updated.
template <typename T>
void CreateBiTree(BiTNode<T> *&Tree)
{
  char ch;
  if (!(cin >> ch))
  {
    Tree = nullptr;
    return;
  } // EOF safety
  if (ch == '#')
  {
    Tree = nullptr;
  }
  else
  {
    Tree = new BiTNode<T>;
    Tree->data = ch;
    Tree->lchild = nullptr;
    Tree->rchild = nullptr;
    CreateBiTree(Tree->lchild);
    CreateBiTree(Tree->rchild);
  }
}

// Recursive postorder (left, right, root)
template <typename T>
void PostOrderRecursive(BiTNode<T> *Tree)
{
  if (!Tree)
    return;
  PostOrderRecursive(Tree->lchild);
  PostOrderRecursive(Tree->rchild);
  cout << Tree->data;
}

// Iterative postorder using one stack and lastVisited pointer
// Works for BiTNode<char>* root (or adapt template if needed)
void PostOrderIterative(BiTNode<char> *root)
{
  if (!root)
    return;
  stack<BiTNode<char> *> st;
  BiTNode<char> *lastVisited = nullptr;
  BiTNode<char> *cur = root;

  while (cur != nullptr || !st.empty())
  {
    // go left as far as possible
    if (cur != nullptr)
    {
      st.push(cur);
      cur = cur->lchild;
    }
    else
    {
      BiTNode<char> *peekNode = st.top();
      // if right child exists and not yet visited, traverse it
      if (peekNode->rchild != nullptr && lastVisited != peekNode->rchild)
      {
        cur = peekNode->rchild;
      }
      else
      {
        // visit node
        cout << peekNode->data;
        lastVisited = peekNode;
        st.pop();
      }
    }
  }
}

int main()
{
  BiTNode<char> *T = nullptr;
  // example input: ABD##E##C#F##  (use '#' for nulls)
  CreateBiTree(T);

  cout << "Recursive postorder: ";
  PostOrderRecursive(T);
  cout << "\nIterative postorder: ";
  PostOrderIterative(T);
  cout << "\n";

  getchar();
  getchar();
  return 0;
}
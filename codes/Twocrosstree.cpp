#include <iostream>

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

class BST {
public:
    TreeNode* root;

    BST() : root(nullptr) {}

    // 插入节点
    TreeNode* insert(TreeNode* node, int val) {
        if (!node) {
            return new TreeNode(val);
        }
        
        if (val < node->val) {
            node->left = insert(node->left, val);
        } else if (val > node->val) {
            node->right = insert(node->right, val);
        }
        
        return node;
    }

    // 删除节点
    TreeNode* deleteNode(TreeNode* root, int e) {
        if (!root) return root;  // 节点不存在
        
        if (e < root->val) {
            root->left = deleteNode(root->left, e);
        } else if (e > root->val) {
            root->right = deleteNode(root->right, e);
        } else {
            // 找到要删除的节点
            
            // 情况1: 没有子节点或只有一个子节点
            if (!root->left) {
                TreeNode* temp = root->right;
                delete root;
                return temp;
            } else if (!root->right) {
                TreeNode* temp = root->left;
                delete root;
                return temp;
            }
            
            // 情况2: 有两个子节点
            // 找到右子树中的最小值节点（中序后继）
            TreeNode* temp = findMin(root->right);
            
            // 用后继节点的值替换当前节点的值
            root->val = temp->val;
            
            // 删除后继节点
            root->right = deleteNode(root->right, temp->val);
        }
        
        return root;
    }

    // 找到最小值节点
    TreeNode* findMin(TreeNode* node) {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }

    // 查找节点
    TreeNode* search(TreeNode* node, int val) {
        if (!node || node->val == val) {
            return node;
        }
        
        if (val < node->val) {
            return search(node->left, val);
        }
        
        return search(node->right, val);
    }

    // 中序遍历
    void inorder(TreeNode* node) {
        if (node) {
            inorder(node->left);
            std::cout << node->val << " ";
            inorder(node->right);
        }
    }
};

int main() {
    BST bst;
    
    // 构建二叉排序树
    bst.root = bst.insert(bst.root, 50);
    bst.root = bst.insert(bst.root, 30);
    bst.root = bst.insert(bst.root, 70);
    bst.root = bst.insert(bst.root, 20);
    bst.root = bst.insert(bst.root, 40);
    bst.root = bst.insert(bst.root, 60);
    bst.root = bst.insert(bst.root, 80);
    
    std::cout << "Original BST (inorder traversal): ";
    bst.inorder(bst.root);
    std::cout << std::endl;
    
    // 删除节点
    int valToDelete;
    std::cout << "Please enter the node value to delete: ";
    std::cin >> valToDelete;
    bst.root = bst.deleteNode(bst.root, valToDelete);
    std::cout << "After deleting " << valToDelete << " (inorder traversal): ";
    bst.inorder(bst.root);
    std::cout << std::endl;
    
    getchar();
    getchar();
    
    return 0;
}
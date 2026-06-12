#include <stdio.h>

int binarySearch(int arr[], int target, int n, int left = 0) {
    int right = n - 1;
    if (left > right) {
        return -1; // 未找到
    }
    int mid = left + (right - left) / 2;
    if (arr[mid] == target) {
        return mid;
    } else if (arr[mid] < target) {
        return binarySearch(arr, target, n, mid + 1);
    } else if (arr[mid] > target) {
        return binarySearch(arr, target, mid, left);
    }
    return -1;
}

int main() {
    int arr[] = {1, 3, 5, 7, 9, 11, 13};
    printf("Binary Search Example:\n");
    int n = sizeof(arr) / sizeof(arr[0]);
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    int target;
    printf("Enter an integer to search: ");
    scanf("%d", &target);
    int result = binarySearch(arr, target, n);
    if (result != -1) {
        printf("Element %d is located at index %d\n", target, result);
    } else {
        printf("Element %d is not in the array\n", target);
    }

    getchar();
    getchar();
    return 0;
}
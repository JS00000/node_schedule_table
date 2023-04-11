// 费用流
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
using namespace std;
#define maxn 600
#define maxm 10007
#define infi (~0u>>1)

class min_cost_flow
{
public:

    int edge_idx;
    int head[maxn];
    struct node
    {
        int to, next, c, w;
    }edges[maxm*10];
    int dis[maxn], load[maxm], pre[maxn];
    bool flag[maxn];
    // dis:距离数组, load:前驱边, pre:前驱点
    // flag:点是否入队

    min_cost_flow() {};

    void add_edge(int u, int v, int c, int w)
    {
        edges[edge_idx].c = c;
        edges[edge_idx].w = w;
        edges[edge_idx].to = v;
        edges[edge_idx].next = head[u];
        head[u] = edge_idx++;
        // reverse edge
        edges[edge_idx].c = 0;
        edges[edge_idx].w = -w;
        edges[edge_idx].to = u;
        edges[edge_idx].next = head[v];
        head[v] = edge_idx++;
    }

    bool spfa(int start, int end, int N)
    {
        int que[maxn*10], qout, qin;
        memset(flag, 0, sizeof(flag));
        memset(load, -1, sizeof(load));
        memset(pre, -1, sizeof(pre));
        for (int i = 0; i <= N; i++) dis[i] = infi;
        qin = qout = 0;
        que[qin++] = start;
        dis[start] = 0;
        flag[start] = 1;
        while(qin != qout)
        {
            int e = que[qout++];
            flag[e] = false;
            for (int k = head[e]; k != -1; k = edges[k].next)
            {
                if (edges[k].c)
                {
                    int ne = edges[k].to;
                    if (dis[ne] - dis[e] > edges[k].w)
                    {
                        dis[ne] = dis[e] + edges[k].w;
                        pre[ne] = e;
                        load[ne] = k;
                        if (!flag[ne])
                        {
                            flag[ne] = 1;
                            que[qin++] = ne;
                        }
                    }
                }
            }
        }
        if (dis[end] == infi) return 0;
        return 1;
    }

    int run(int start, int end, int N)
    {
        int u, mini, ans_flow = 0, ans_cost = 0;
        while(spfa(start, end, N))
        {
            u = end;
            mini = infi;
            while(pre[u] != -1)
            {
                mini = min(mini, edges[load[u]].c);
                u = pre[u];
            }
            u = end;
            while(pre[u] != -1)
            {
                edges[load[u]].c -= mini;
                edges[load[u]^1].c += mini;
                u = pre[u];
            }
            ans_cost += dis[end]*mini;
            ans_flow += mini;
        }
        return ans_flow;
    }

};


#ifndef SRC_PLACEMENT_SYSTEM_HPP_
#define SRC_PLACEMENT_SYSTEM_HPP_

#include <iostream>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <list>
#include <queue>
#include <map>
#include <exception>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <climits>
namespace placement::backend {

// Graph
struct Cell {
    std::vector<std::shared_ptr<Cell>> adjacency_list;
    std::shared_ptr<Cell> parent;
    std::shared_ptr<Cell> next = NULL;
    int gain;
    std::string name;
    int x = 0,  y = 0;  // left corner coordinate (glabol placement)
    int width = 0;
    int height = 0;
    int area = 0;
    int id;  // global cell id
    int weight = 1;

    int final_x = 0,  final_y = 0;
};


struct Terminal {
    std::string name;
    int x = 0,  y = 0;  // left corner coordinate
    int width = 0;
    int height = 0;
};

/* Spindler, P., Schlichtmann, U., & Johannes, F.M. (2008).
   Abacus: fast legalization of standard cell circuits with minimal movement.
   ACM International Symposium on Physical Design.*/

// abacus dynamic program
struct Cluster {
    explicit Cluster(int x)
    :xc{x}, ec{0}, qc{0}, wc{0} {}
    int xc;  // start x-position
    int ec;  // ec = ec + e(i)
    int qc;  // qc = qc +e(i)[x'(i) -wc]
    int wc;  // wc = wc + w(i)

    std::vector<std::shared_ptr<Cell>> cell_list;
    void addCell(std::shared_ptr<Cell> c) {
        cell_list.push_back(c);
        ec += c->weight;
        qc += c->weight * (c->x - wc);
        wc += c->width;
    }

    void addCluster(Cluster* cluster) {
        for (const auto& cell : cluster->cell_list)
            cell_list.push_back(cell);
        ec += cluster->ec;
        qc += (cluster->qc - cluster->ec * wc);
        wc += cluster->wc;
    }
};


struct Subrow {
    Subrow(int start_x, int width, int y)
    : x1{start_x}, x2{start_x + width}, y{y} {
        remain_space = x2 - x1;
        cost = 0;
        last_cluster_num = 0;
        last_backup_num = 0;
    }

    int x1, x2;
    int remain_space;
    int y;
    int cost;
    int last_cluster_num;  // point to last Cluster in Clusters.
    int last_backup_num;  // point to last Cluster in Backup
    std::vector<Cluster> cluster_list;
    std::vector<Cluster> cluster_backup_list;
    int modifiedX(int x1, int x2, const std::shared_ptr<Cell>& cell) {
        if (cell->x < x1) {
            return x1;
        }
        if (cell->x + cell->width > x2) {
            return x2 - cell->width;
        }
        return cell->x;
    }

    bool isEmpty() {
        return last_cluster_num == 0;
    }

    Cluster& last() {
        if (isEmpty()) {
            throw std::invalid_argument("cluster_list is empty");
        }
        return cluster_list[last_cluster_num - 1];
    }

    void appendCluster(int x) {
        if (cluster_list.size() == last_cluster_num)
            cluster_list.push_back(Cluster(x));
        else if (cluster_list.size() > last_cluster_num)
            cluster_list[last_cluster_num] = Cluster(x);

        last_cluster_num++;
    }

    void collapse() {
        int c = last_cluster_num - 1;
        for (; c >= 0; c--) {
            auto& cluster = cluster_list[c];
            cluster.xc = cluster.qc / cluster.ec;
            if (cluster.xc < x1)
                cluster.xc = x1;
            if (cluster.xc > x2 - cluster.wc)
                cluster.xc = x2 - cluster.wc;
            if (c > 0 && cluster_list[c-1].xc + cluster_list[c-1].wc > cluster.xc)
                cluster_list[c-1].addCluster(&cluster);
            else
                break;
        }
        last_cluster_num = c + 1;
    }

    void place(const std::shared_ptr<Cell>& cell) {
        int modify_x = modifiedX(x1, x2, cell);
        if (isEmpty() || last().xc + last().wc <= modify_x) {
            appendCluster(modify_x);
            last().addCell(cell);
        } else {
            last().addCell(cell);
            collapse();
        }

        // std::cout << cell->name <<" " << modify_x <<" " <<last_cluster_num  << " " << cost <<  std::endl;
        // fgetc(stdin);
    }

    int getPosition() {
        int cost = 0;
        for (int i = 0; i < last_cluster_num; ++i) {
            const auto& cluster = cluster_list[i];
            int x = cluster.xc;
            for (auto& cell : cluster.cell_list) {
                cell->final_x = x;
                cell->final_y = y;
                cost += std::abs(cell->final_x - cell->x);
                cost += std::abs(cell->final_y - cell->y);
                x += cell->width;
            }
        }
        return cost;
    }

    void backup() {
        if (last_cluster_num > cluster_backup_list.size())
            cluster_backup_list.push_back(cluster_list.back());

        last_backup_num = last_cluster_num;
        cluster_backup_list[last_backup_num - 1] = cluster_list[last_cluster_num - 1];
    }

    void recoverClusterList() {
        // std::cout << last_backup_num << " " << last_cluster_num << std::endl;
        // fgetc(stdin);
        if (last_cluster_num == last_backup_num) {
            cluster_list[last_cluster_num - 1] = cluster_backup_list[last_cluster_num - 1];
        } else if (last_cluster_num < last_backup_num) {
            for (int i = last_cluster_num - 1; i < last_backup_num; ++i)
                cluster_list[i] = cluster_backup_list[i];
        }
        last_cluster_num = last_backup_num;
    }

};


struct Row {
    Row(int x, int y, int w, int h)
    :y{y}, height{h} {
        subrow_list.emplace_back(x, w, y);
    }

    int y;
    int height;
    std::vector<Subrow> subrow_list;
    std::pair<Subrow*, int> placeRow(const std::shared_ptr<Cell>& cell);
    void block(Terminal& terminal);

    int calCost() {
        int cost = 0;
        for (auto& subrow : subrow_list) {
            cost += subrow.getPosition();
        }
        return cost;
    }
};

/*system infomation*/
struct System {
    int chip_width;
    int chip_height;
    int row_height;
    int num_rows;
    int num_terminals;
    int num_cells;
    int total_cell_area;
    int max_cell_area;
    std::vector<std::shared_ptr<Terminal>> terminal_list;
    std::vector<std::shared_ptr<Cell>> cell_list;
    std::vector<std::shared_ptr<Cell>> left_cell_list;
    std::vector<std::shared_ptr<Cell>> right_cell_list;
    std::vector<Row> row_list;
    int partition_cost = 0;  // max cut
    int legalization_cost = 0;
};

// user need to enter terminal with the increasing x order.
// the block function check the relative position between termianl node and place-row ,
// and make the  correction : split the subrows or boundary correction.
void Row::block(Terminal& terminal) {
    // re-checking y range
    if (terminal.y + terminal.height <= y  || terminal.y >= y + height) return;
    Subrow* last_ptr = &(*subrow_list.rbegin());

    int overlap_condition;
    int t_x1 = terminal.x;
    int t_x2 = t_x1 + terminal.width;
    if ((t_x2 <= last_ptr->x1 )  || ( t_x1 >= last_ptr->x2) ) overlap_condition = 0;
    else if (t_x1 <= last_ptr->x1  && t_x2 >= last_ptr->x2) overlap_condition = 1;
    else if (t_x1 <= last_ptr->x1 && t_x2 < last_ptr->x2) overlap_condition = 2;
    else if (t_x1 > last_ptr->x1 && t_x2 < last_ptr->x2) overlap_condition = 3;
    else if (t_x1 > last_ptr->x1 && t_x2 >= last_ptr->x2) overlap_condition = 4;
    if (overlap_condition == 1) {
        subrow_list.pop_back();  // delete subrow
    } else if (overlap_condition == 2) {
        last_ptr->x1 = t_x2;
    } else if (overlap_condition == 3) {
        // split new subrow
        subrow_list.push_back({t_x2, last_ptr->x2-t_x2, y});
        last_ptr = &subrow_list[subrow_list.size() - 2];  // push back may allocate new memory
        last_ptr->x2 = t_x1;
    } else if (overlap_condition == 4) {
        last_ptr->x2 = t_x1;
    }


    if (overlap_condition > 1)
        last_ptr->remain_space = last_ptr->x2 - last_ptr->x1;
}


std::pair<Subrow*, int> Row::placeRow(const std::shared_ptr<Cell>& cell) {
    Subrow* subrow = nullptr;
    int best_cost = INT_MAX;
    // binary Search Subrow
    int left = 0;
    int right = subrow_list.size()-1;
    int start_id = std::max(0, left);
    while (left < right) {
        int mid = (left + right)/2;
        if (subrow_list[mid].x1 == cell->x) {
            start_id = mid;
            break;
        } else if (subrow_list[mid].x1 > cell->x) {
            right = mid-1;
        } else {
            left = mid+1;
        }
    }

    auto attempPlaceSubrow = [] (Subrow& subrow, std::shared_ptr<Cell> cell,
    int& best_cost, Subrow* &best_subrow_place) ->bool {
        if (subrow.remain_space >= cell->width) {
            subrow.place(cell);
            int after_cost = subrow.getPosition();
            int delta_cost = after_cost - subrow.cost;

            subrow.recoverClusterList();
            if (delta_cost < best_cost) {
                best_subrow_place = &subrow;
                best_cost = delta_cost;
                return true;
            } else {
                return false;
            }
        }
        return true;
    };

    // std::cout << cell->name << " " << start_id << std::endl;
    // fgetc(stdin);
    for (int i = start_id - 1; i <= start_id + 1; ++i) {
        if (i >= 0 && i < subrow_list.size()) {
            attempPlaceSubrow(subrow_list[i], cell, best_cost, subrow);
        }
    }

    for (int i = start_id - 2; i >= 0; --i) {
        if (i >= 0)
            if (!attempPlaceSubrow(subrow_list[i], cell, best_cost, subrow))
                break;
    }

    for (int i = start_id + 2; i <subrow_list.size(); ++i) {
        if (i < subrow_list.size())
            if (!attempPlaceSubrow(subrow_list[i], cell, best_cost, subrow))
                break;
    }

    // std::cout << subrow->x1 <<" " << subrow->x2 << " " << subrow->remain_space << std::endl;
    // std::cout << best_cost << std::endl;
    // fgetc(stdin);

    return std::make_pair(subrow, best_cost);
}

}  // namespace placement::backend
#endif  // SRC_PLACEMENT_SYSTEM_HPP_


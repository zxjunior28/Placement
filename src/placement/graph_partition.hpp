#ifndef SRC_PLACEMENT_GRAPH_PARTITION_HPP_
#define SRC_PLACEMENT_GRAPH_PARTITION_HPP_

#include <placement/system.hpp>

namespace placement {

/*Graph Partition by Fiduccia Matteyses method*/
class GraphPartition {
 public:
    using system_ptr_type = std::shared_ptr<backend::System>;
    using cell_ptr_type = std::shared_ptr<backend::Cell>;
    explicit GraphPartition(system_ptr_type system_ptr)
    : system_ptr_(system_ptr) {}
    ~GraphPartition() = default;

    void initialize();
    system_ptr_type FMpartition(int max_iter);

 private:
    system_ptr_type system_ptr_;
    std::vector<int> bit_vector_;  // chip
    std::vector<int> best_bit_vector_;  // chip
    std::vector<cell_ptr_type> left_buckets_;
    std::vector<cell_ptr_type> right_buckets_;
    int left_area_;
    int right_area_;
    std::unordered_map<std::string, bool> locked_cell_list_;
    int max_degree_;
    int max_left_gain_index_ = 0;
    int max_right_gain_index_ = 0;
    bool current_side_ = 0;  // default: left side

    void createGraph();
    size_t calCost();
    void getBothSideArea();
    bool maxCutPartition(size_t& cost);
        void initializeBuckets();
        void initializeGain();
            std::function<bool(cell_ptr_type, cell_ptr_type)> isInAdjacencyList
            = [&] (const cell_ptr_type c1, const cell_ptr_type c2) {
                for (const auto& c : c1->adjacency_list)
                    if (c == c2) return true;
                return false;
            };
            void sortBucket(const cell_ptr_type& cell, int gain, const std::string& side);
        void initializeLockCells();
        void updateGain();
            void reductGain(cell_ptr_type cell, std::vector<cell_ptr_type> insert_buckets);
            void increaseGain(cell_ptr_type cell, std::vector<cell_ptr_type> curr_buckets);


};


void GraphPartition::initialize() {
    /*Create Graph by the overlap relationship*/
    createGraph();

    /*initialize bit vector (Group)*/
    int num_cells = system_ptr_->num_cells;
    int mid = num_cells / 2;
    bit_vector_.resize(num_cells);
    for (int i = 0; i < num_cells; ++i) {
        const auto& cell = system_ptr_->cell_list[i];
        if (i < mid) {
            bit_vector_[i] = 0;  // left chip idx
        } else {
            bit_vector_[i] = 1;  // right chip idx
        }
    }

    max_degree_ = 0;
    for (const auto&  cell : system_ptr_->cell_list) {
        max_degree_ = std::max(max_degree_,
                      static_cast<int>(cell->adjacency_list.size()));
    }
}

/*Fiduccia Matteyses method(F-M algorithm)*/
GraphPartition::system_ptr_type GraphPartition::FMpartition(int max_iter) {
    int iter = 0;
    size_t cost = calCost();

    auto init_group_list = bit_vector_;
    while (iter < max_iter) {
        // std::cout << "iter == " << iter << std::endl;

        /*random sort*/
        std::random_device rd;
        std::default_random_engine rng(rd());
        std::shuffle(bit_vector_.begin(), bit_vector_.end(), rng);

        getBothSideArea();

        /*max-cut partition*/
        maxCutPartition(cost);
        iter++;

    }

    // write data in left & right
    const auto& cell_list = system_ptr_->cell_list;
    const auto& num_cells = system_ptr_->num_cells;
    system_ptr_->left_cell_list.reserve(num_cells);
    system_ptr_->right_cell_list.reserve(num_cells);
    system_ptr_->partition_cost = cost;

    if (!best_bit_vector_.empty()) {
        for (size_t i = 0; i < best_bit_vector_.size(); ++i) {
            if (best_bit_vector_[i] == 0) {
                cell_list[i]->id = 0;
                system_ptr_->left_cell_list.push_back(cell_list[i]);
            } else {
                cell_list[i]->id = 1;
                system_ptr_->right_cell_list.push_back(cell_list[i]);
            }
        }
    } else {
        for (size_t i = 0; i < init_group_list.size(); ++i) {
            if (init_group_list[i] == 0) {
                cell_list[i]->id = 0;
                system_ptr_->left_cell_list.push_back(cell_list[i]);
            } else {
                cell_list[i]->id = 1;
                system_ptr_->right_cell_list.push_back(cell_list[i]);
            }
        }
    }
    return std::move(system_ptr_);
}


/*******************************
/*
/*    Private Implemantation
/*
/*******************************/

void GraphPartition::getBothSideArea() {
    left_area_ = 0;
    right_area_ = 0;
    for (int i = 0; i < bit_vector_.size(); ++i) {
        if (bit_vector_[i] == 0) {
            left_area_ += system_ptr_->cell_list[i]->area;
        } else {
            right_area_+= system_ptr_->cell_list[i]->area;
        }
    }
}


// plz modified!!
// Double counting of area

/*Create Graph by the overlap relationship*/
void GraphPartition::createGraph() {
    auto isOverlapping = [] (const cell_ptr_type& c1, const cell_ptr_type& c2) -> bool {
        if (c2->x < c1->x + c1->width && c2->x + c2->width > c1->x
            && c2->y < c1->y + c1->height && c2->y + c2->height > c1->y)
            return true;
        else
            return false;
    };

    /*Adjacency list*/
    // please sort first!!!! this step can accelerate
    auto cell_list = system_ptr_->cell_list;
    std::sort(cell_list.begin(), cell_list.end(), [&](const cell_ptr_type c1, const cell_ptr_type c2){
        return c1->x < c2->x;
    });

    for (int i = 0; i < system_ptr_->num_cells-1; ++i) {
        for (int j = i + 1; j < system_ptr_->num_cells; ++j) {
                if (cell_list[j]->x > cell_list[i]->x + cell_list[i]->width)
                  break;
                if (isOverlapping(cell_list[i], cell_list[j])) {
                   cell_list[i]->adjacency_list.push_back(cell_list[j]);
                   cell_list[j]->adjacency_list.push_back(cell_list[i]);
                }
        }
    }
}

size_t GraphPartition::calCost() {
    size_t cost = 0;
    const auto& cell_list = system_ptr_->cell_list;
    /*check cell in left(0)*/
    for (int i = 0; i < bit_vector_.size(); ++i) {
        if (bit_vector_[i] == 0) {
            /*check where adjacency cells are*/
            for (const auto& cell : cell_list[i]->adjacency_list)
                if (bit_vector_[cell->id] == 1)
                    cost++;
        }
    }
    return cost;
}

bool GraphPartition::maxCutPartition(size_t& cost) {
    // std::cout << "hi!!1" << std::endl;
    //  initialize the size of buckets on both side
    initializeBuckets();
    //  initialize gain
    initializeGain();
    // initialize locked cells
    initializeLockCells();

    /// Set which side to begin, to maintain balance
    int num_zeros = 0;
    for (bool is_cell_right : bit_vector_) {
        if (is_cell_right)
            num_zeros++;
    }
    if (num_zeros > 0 && bit_vector_.size() / num_zeros < 2)
        current_side_ = true;

    
    int iter = 0, same = 0;
    int best_cost = cost;
 
    while (iter < system_ptr_->num_cells) {
        // std::cout << "<====================>" << std::endl;
        // std::cout << "  -- " << iter << std::endl;
        updateGain();
        int temp_cost = calCost();


        if (temp_cost > best_cost) {
            best_cost = temp_cost;
            best_bit_vector_ = bit_vector_;
            same = 0;
        } else {
            same++;
        }
        // std::cout << " - temp_cost= " << temp_cost << " best_cost= " << best_cost << std::endl;
        // std::cout << "<====================>\n" << std::endl;


        iter++;

         if (same > 3)
                break;
    }

    // max_cut
    if (best_cost > cost) {
        cost = best_cost;
    }
    return true;
}

void GraphPartition::initializeBuckets() {
    // max_degree_ = 0;
    // for (const auto&  cell : system_ptr_->cell_list) {
    //     max_degree_ = std::max(max_degree_,
    //                   static_cast<int>(cell->adjacency_list.size()));
    // }
    left_buckets_.clear();
    right_buckets_.clear();

    left_buckets_.reserve(2*max_degree_ + 1);
    right_buckets_.reserve(2*max_degree_ + 1);
    for (int i = 0; i < 2*max_degree_ + 1; ++i) {
        left_buckets_.push_back(std::make_shared<backend::Cell>());
        right_buckets_.push_back(std::make_shared<backend::Cell>());
    }
}

// calculating the initial gain for each cell
void GraphPartition::initializeGain() {
    // std::cout <<"initializeGain" << std::endl;
    // isInAdjacencyList
    int gain;
    const auto& num_cells =  system_ptr_->num_cells;
    const auto& cell_list = system_ptr_->cell_list;
    for (int i = 0; i < num_cells; ++i) {
        gain = 0;
        for (const auto& adjacency_cell : cell_list[i]->adjacency_list) {
            int index = adjacency_cell->id;
            if (bit_vector_[i] == bit_vector_[index])
                gain++;
            else
                gain--;
        }
        // fgetc(stdin);
        //  std::cout <<"initializeGain3" << std::endl;

        if (bit_vector_[i] == 0) {
            sortBucket(cell_list[i], gain, "left");
        } else {
            sortBucket(cell_list[i], gain, "right");
        }
    }

    // for (int i = 0; i < right_buckets_.size(); ++i) {
    //     std::cout << right_buckets_[i]->gain  << " " << left_buckets_[i]->gain << std::endl;
    // }

    // fgetc(stdin);

    // std::cout <<"initializeGain2" << std::endl;
}

void GraphPartition::sortBucket(const cell_ptr_type& cell, int gain, const std::string& side) {
    int* max_gain_idx = nullptr;
    int index = 0;

    if (side == "left") {
        max_gain_idx = &max_left_gain_index_;
    } else if (side == "right") {
        max_gain_idx = &max_right_gain_index_;
    }

    if (gain != 0) {
        index = max_degree_ + gain;
        *max_gain_idx = std::max(index, *max_gain_idx);
    } else {
        index = max_degree_;
        *max_gain_idx = std::max(index, *max_gain_idx);
    }


    cell_ptr_type cell2 = nullptr;
    if (side == "left") {
        cell2 = left_buckets_[index];
    } else if (side == "right") {
        cell2 = right_buckets_[index];
    }

    // set gain and gain link list
    auto temp = cell2->next;
    cell2->next = cell;
    cell2->next->gain = gain;
    cell2->next->next = temp;

    // std::cout << cell2->next->gain << " " << cell2->next->name << std::endl;
    // fgetc(stdin);
}

void GraphPartition::initializeLockCells() {
    locked_cell_list_.reserve(system_ptr_->num_cells);
    for (const auto& cell : system_ptr_->cell_list)
        locked_cell_list_[cell->name] = false;
}

void GraphPartition::updateGain() {
    cell_ptr_type max_gain_cell = nullptr;
    std::vector<cell_ptr_type> cur_buckets, insert_buckets;
    int *cur_area, *insert_area;
    int max_gain_index;

    /*left*/
    if (current_side_ == 0) {
        cur_buckets = left_buckets_;
        insert_buckets = right_buckets_;
        max_gain_index = max_left_gain_index_;
        cur_area = &left_area_;
        insert_area = &right_area_;
    } else {
        /*right*/
        cur_buckets = right_buckets_;
        insert_buckets = left_buckets_;
        max_gain_index = max_right_gain_index_;
        cur_area = &right_area_;
        insert_area = &left_area_;
    }

    bool found = false;
    int i = 0;
    /*search */
    while (!found) {
        cell_ptr_type cell_ptr = cur_buckets[max_gain_index - i];

        while (cell_ptr) {
            if (!cell_ptr->name.empty() && !locked_cell_list_[cell_ptr->name]) {
                max_gain_cell = cell_ptr;
                found = true;
                break;
            }
            // link list
            cell_ptr = cell_ptr->next;
        }
        ++i;
        if (i >= max_degree_)
            break;
    }

    // 1. checking balance by flipping sides
    if (!max_gain_cell) {
        current_side_ = !current_side_;
        return;
    }

    // 1. checking check area balance by flipping
    *cur_area -= max_gain_cell->area;
    *insert_area += max_gain_cell->area;

    double upper_limit = system_ptr_->total_cell_area*0.5 + system_ptr_->max_cell_area;
    double lower_limt = system_ptr_->total_cell_area*0.5 - system_ptr_->max_cell_area;

    if (lower_limt <= 0)
        lower_limt = system_ptr_->total_cell_area*0.5 - system_ptr_->max_cell_area;

    if (static_cast<double>(*cur_area) < lower_limt
        || static_cast<double>(*cur_area) > upper_limit) {
        current_side_ = !current_side_;
        *cur_area += max_gain_cell->area;
        *insert_area -= max_gain_cell->area;
        return;
    }


    // change max_gain_cell side
    bit_vector_[max_gain_cell->id] = !bit_vector_[max_gain_cell->id];

    // insert
    //  Lock cell
    locked_cell_list_[max_gain_cell->name] = true;

    for (auto& cell : max_gain_cell->adjacency_list) {
        if (!locked_cell_list_[cell->name]) {
            if (bit_vector_[cell->id] == current_side_) {
                reductGain(cell, cur_buckets);
            } else {
                increaseGain(cell, insert_buckets);
            }
        }
    }

    // fgetc(stdin);

    current_side_ = !current_side_;
    return;
}

void GraphPartition::reductGain(cell_ptr_type cell,
std::vector<cell_ptr_type> insert_buckets) {
    int* max_gain_ptr = nullptr;
    if (current_side_) {
        max_gain_ptr = &max_left_gain_index_;
    } else {
        max_gain_ptr = &max_right_gain_index_;
    }

    //  Remove cell
    auto remove_ptr = insert_buckets[max_degree_ + cell->gain];

    // std::cout << *max_gain_ptr<<" "<< cell->name<< " "<< cell->gain <<" " << remove_ptr->name<< std::endl;

    while (remove_ptr) {
        auto prev = remove_ptr;
        remove_ptr = remove_ptr->next;
        // Remove link list
        if (remove_ptr == cell) {
            prev->next = cell->next;
            cell->next = nullptr;
            break;
        }
    }

    // Add cell
    auto prev_idx = insert_buckets[max_degree_ + cell->gain - 2];
    auto temp = prev_idx->next;
    prev_idx->next = cell;
    if (insert_buckets[max_degree_ + cell->gain]->next == nullptr && *max_gain_ptr == max_degree_ + cell->gain)
        *max_gain_ptr -= 1;
    cell->gain -= 2;
    prev_idx->next->next = temp;

}

void GraphPartition::increaseGain(cell_ptr_type cell,
std::vector<cell_ptr_type> curr_buckets) {
    int* max_gain_ptr = nullptr;
    if (current_side_) {
        max_gain_ptr = &max_left_gain_index_;
    } else {
        max_gain_ptr = &max_right_gain_index_;
    }

    //  Remove cell
    cell_ptr_type remove_ptr = curr_buckets[max_degree_ + cell->gain];

    // std::cout << *max_gain_ptr<<" "<< cell->name<< " "<< cell->gain <<" " << remove_ptr->name<< std::endl;
    while (remove_ptr) {
        auto prev = remove_ptr;
        remove_ptr = remove_ptr->next;
        if (remove_ptr == cell) {  // Remove
            prev->next = cell->next;
            cell->next = nullptr;
            break;
        }
    }

    // Add cell
    auto next_idx = curr_buckets[max_degree_ + cell->gain + 2];
    auto temp = next_idx->next;
    next_idx->next = cell;
    cell->gain += 2;
    *max_gain_ptr = std::max(*max_gain_ptr, max_degree_ + cell->gain);
    next_idx->next->next = temp;

}


}  // namespace placement

#endif  // SRC_PLACEMENT_GRAPH_PARTITION_HPP_

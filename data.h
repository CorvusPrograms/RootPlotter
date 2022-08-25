#include "hist_def.h"
#include "TH1.h"

class DataSetOperator{
    std::vector<SourceSet::IdType> sets;
    virtual const std::vector<SourceSet::IdType>& getSets() const {return sets;} 
    virtual void setSets(std::vector<SourceSet::IdType>& s){ sets = s;}
    virtual float produce(TH1* hist) {return 0 ;} 
    virtual void apply(TH1* hist) {}
    virtual ~DataSetOperator() = default;
};

    
class Integral : public DataSetOperator {
public:
    float produce(TH1* h) override { op->return h->Integral();}
    virtual ~Integral() = default;
};


class Normalizer: public HistManipulator{
    virtual ~Normalizer() = default;
    float norm_to = 1;
    DataSetOperator* op = nullptr;
    Normalizer(float to):norm_to{to}{}
    Normalizer(DataSetOperator* op): op{op}
    void apply(TH1* hist) override {
        if (op) {
            norm_to = op->produce(hist);
        }
        hist->Scale(norm_to / hist->Integral());
    };
};




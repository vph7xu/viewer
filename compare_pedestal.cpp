struct APVAddress
{
    int crate_id, mpd_id, adc_ch;

    APVAddress():crate_id(-1), mpd_id(-1), adc_ch(-1)
    {}

    APVAddress(const APVAddress &addr):crate_id(addr.crate_id),
    mpd_id(addr.mpd_id), adc_ch(addr.adc_ch)
    {}

    APVAddress & operator=(const APVAddress &addr) {
        crate_id = addr.crate_id, mpd_id = addr.mpd_id, adc_ch = addr.adc_ch;
        return *this;
    };

    bool operator==(const APVAddress &addr) const
    {
        return (crate_id == addr.crate_id) && (mpd_id == addr.mpd_id) && (adc_ch == addr.adc_ch);
    }
};

namespace std {
    template<> struct hash<APVAddress>
    {
        std::size_t operator()(const APVAddress &k) const
        {
            return ( (k.adc_ch & 0xf)
                    | ((k.mpd_id & 0x7f) << 4)
                    | ((k.crate_id & 0xff)<<11)
                   );
        }
    };
}

// obsolete
unordered_map<APVAddress, TH1F*> GetPedestalDistribution(const char* file)
{
    unordered_map<APVAddress, TH1F*> res;

    fstream f(file, iostream::in);
    if(!f.is_open()) cout<<"Error: cannot open file: "<<file<<endl;

    return res;
}

// a helper
void draw_pad(TH1F *h1, TH1F* h2, TCanvas* c, int npad, int pos)
{
    c->cd(npad);
    gStyle -> SetOptStat(1);

    h1 -> SetLineColor(2);
    h1 -> SetLineWidth(1);
    h1 -> Draw();
    gPad -> Update();

    h2 -> SetLineColor(4);
    h2 -> SetLineWidth(1);
    h2 -> Draw();
    gPad -> Update();

    TH1F *h3 = (TH1F*)h1 -> Clone("h_diff");
    h3 -> Add(h2, -1);
    h3 -> SetLineColor(1);
    //h3 -> SetLineStyle(2);
    h3 -> SetFillStyle(1001);
    if(pos == 0 || pos == 1) {
        h3 -> Draw();
        gPad -> Update();
    }

    THStack *hs = new THStack();
    hs -> SetTitle(h1->GetTitle());
    hs -> Add(h1);
    hs -> Add(h2);
    if(pos == 0 || pos == 1) {
        hs -> Add(h3);
    }
    hs -> Draw("nostack");
    hs -> GetXaxis() -> SetTitle(h1->GetXaxis()->GetTitle());
    hs -> GetYaxis() -> SetTitle(h1->GetYaxis()->GetTitle());

    TLegend *leg = new TLegend(0.6, 0.64, 0.88, 0.85);
    leg -> AddEntry(h1, "Divider run 876", "lp");
    leg -> AddEntry(h2, "PS run 881", "lp");
    leg -> AddEntry(h3, "Diff: old - new", "lp");
    leg -> Draw();

    // draw stats
    gPad -> Update();
    TPaveStats* st1 = (TPaveStats*)h1 -> FindObject("stats");
    st1 -> GetLineWith("Mean") -> SetTextColor(2);
    st1 -> GetLineWith("Entries") -> SetTextColor(2);
    st1 -> GetLineWith("Std Dev") -> SetTextColor(2);
    TPaveStats* st2 = (TPaveStats*)h2 -> FindObject("stats");
    TPaveStats* st3;
    if(pos == 0 || pos == 1)
        st3 = (TPaveStats*)h3 -> FindObject("stats");

    if( pos == 0) // noise
    {
        st1 -> SetX1NDC(0.2); st1 -> SetY1NDC(0.52);
        st1 -> SetX2NDC(0.5); st1 -> SetY2NDC(0.68);
        st1 -> Draw();

        st2 -> SetX1NDC(0.2); st2 -> SetY1NDC(0.72);
        st2 -> SetX2NDC(0.5); st2 -> SetY2NDC(0.88);
        st2 -> Draw();

        st3 -> SetX1NDC(1.0); st3 -> SetY1NDC(1.0);
        st3 -> SetX2NDC(1.0); st3 -> SetY2NDC(1.0);
        st3 -> Draw();
    }
    else if(pos == 1) { // offset
        st1 -> SetX1NDC(0.2); st1 -> SetY1NDC(0.2);
        st1 -> SetX2NDC(0.5); st1 -> SetY2NDC(0.4);
        st1 -> Draw();

        st2 -> SetX1NDC(0.52); st2 -> SetY1NDC(0.2);
        st2 -> SetX2NDC(0.82); st2 -> SetY2NDC(0.4);
        st2 -> Draw();

        st3 -> SetX1NDC(1.0); st3 -> SetY1NDC(1.0);
        st3 -> SetX2NDC(1.0); st3 -> SetY2NDC(1.0);
        st3 -> Draw();
    }
    else if(pos == 2) { // noise overall
        st1 -> SetX1NDC(0.2); st1 -> SetY1NDC(0.52);
        st1 -> SetX2NDC(0.5); st1 -> SetY2NDC(0.68);
        st1 -> Draw();

        st2 -> SetX1NDC(0.2); st2 -> SetY1NDC(0.72);
        st2 -> SetX2NDC(0.5); st2 -> SetY2NDC(0.88);
        st2 -> Draw();
    }
    else if(pos == 3) { // offset overall
        st1 -> SetX1NDC(0.15); st1 -> SetY1NDC(0.52);
        st1 -> SetX2NDC(0.4); st1 -> SetY2NDC(0.68);
        st1 -> Draw();

        st2 -> SetX1NDC(0.15); st2 -> SetY1NDC(0.72);
        st2 -> SetX2NDC(0.4); st2 -> SetY2NDC(0.88);
        st2 -> Draw();
    }
    

    gPad -> Modified();
    gPad -> Update();
}

void compare_pedestal(const char *file1="../database/gem_ped_876.root",
        const char* file2="../database/gem_ped_881.root")
{
    TFile *f1 = new TFile(file1);
    TFile *f2 = new TFile(file2);

    //TIter keyList(f1 -> GetListOfKeys());
    //TKey *key;
    //int nAPV = 0;

    // set up canvas
    TCanvas *c = new TCanvas("c", "noise", 1900, 1000);
    c -> Divide(4, 4);
    TCanvas *c1 = new TCanvas("c1", "offset", 1900, 1000);
    c1 -> Divide(4, 4);
    TCanvas *c2 = new TCanvas("c2", "overall", 1900, 1000);
    c2 -> Divide(2, 2);

    for(int i=0; i<16; i++) {
        // noise
        TH1F *h1 = (TH1F*) f1 -> Get(Form("h_noise_crate_%d_fec_%d_adc_%d", 2, 0, i));
        TH1F *h2 = (TH1F*) f2 -> Get(Form("h_noise_crate_%d_fec_%d_adc_%d", 2, 0, i));
        draw_pad(h1, h2, c, i+1, 0);

        // offset
        TH1F *h3 = (TH1F*) f1 -> Get(Form("h_offset_crate_%d_fec_%d_adc_%d", 2, 0, i));
        TH1F *h4 = (TH1F*) f2 -> Get(Form("h_offset_crate_%d_fec_%d_adc_%d", 2, 0, i));
        draw_pad(h3, h4, c1, i+1, 1);
    }

    //overall
    TH1F* h_x_noise_1 = (TH1F*) f1 -> Get(Form("h_overall_x_noise_chamber_%d", 13));
    TH1F* h_y_noise_1 = (TH1F*) f1 -> Get(Form("h_overall_y_noise_chamber_%d", 13));
    TH1F* h_x_offset_1 = (TH1F*) f1 -> Get(Form("h_overall_x_offset_chamber_%d", 13));
    TH1F* h_y_offset_1 = (TH1F*) f1 -> Get(Form("h_overall_y_offset_chamber_%d", 13));

    TH1F* h_x_noise_2 = (TH1F*) f2 -> Get(Form("h_overall_x_noise_chamber_%d", 13));
    TH1F* h_y_noise_2 = (TH1F*) f2 -> Get(Form("h_overall_y_noise_chamber_%d", 13));
    TH1F* h_x_offset_2 = (TH1F*) f2 -> Get(Form("h_overall_x_offset_chamber_%d", 13));
    TH1F* h_y_offset_2 = (TH1F*) f2 -> Get(Form("h_overall_y_offset_chamber_%d", 13));

    draw_pad(h_x_noise_1, h_x_noise_2, c2, 1, 2);
    draw_pad(h_y_noise_1, h_y_noise_2, c2, 2, 2);
    draw_pad(h_x_offset_1, h_x_offset_2, c2, 3, 3);
    draw_pad(h_y_offset_1, h_y_offset_2, c2, 4, 3);

    c -> Print("pedestal_compare.pdf(");
    c1 -> Print("pedestal_compare.pdf");
    c2 -> Print("pedestal_compare.pdf)");
}
